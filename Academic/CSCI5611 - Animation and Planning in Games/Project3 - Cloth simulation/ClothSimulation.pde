// Node struct
class Node {
  Vec3 basePos;
  Vec3 pos;
  Vec3 vel;
  Vec3 lastPos;
  ArrayList<Integer> neighbors;

  Node(Vec3 pos) {
    this.basePos = pos;
    this.pos = pos;
    this.vel = new Vec3(0, 0, 0);
    this.lastPos = pos;
  }
}

// Link length, thickness
float linkLength = 0.1;
float linkLengthD = 0.141421;
float thickness = 0.08;

// Gravity
Vec3 gravity = new Vec3(0, 10, 0);

// wind
Vec3 wind = new Vec3(0, 0, 0);
float airDensity = 18.0;
float dragCoefficient = 10.0;

// Scaling factor for the scene
float sceneScale = width / 10.0f;

// Physics Parameters
int numRelaxationSteps = 10;
int numSubsteps = 3;
int fRate = 60;
float damping = 0.6;

float maxDist = linkLength * fRate * numSubsteps;

int numRows = 30;
int numColumns = 20;
int numNodes = numRows * numColumns;

// Nodes
Vec3 basePos = new Vec3(5, 5.6, 5);

//Other scene objects
Vec3 earthPos = new Vec3(6.5, 5, 5.9);
float earthRadius = 0.4;
float earthRadiusC = 0.42;
ArrayList<Node> nodes = new ArrayList<Node>();

//textures
PImage bg;
PImage earth;
PImage tex;
PImage boxTex;

//time
float time = 0;

//Toggles
boolean paused = true;
boolean stepMode = false;
boolean meshMode = false;
boolean directionalLight = true;
boolean pinned = true;
boolean handleCollision = true;
boolean handleDrag = true;
boolean diagonalConstraints = false;

//camera
Camera camera;

//Simulation functions
void update_physics(float dt) {
  if(handleDrag) {
    add_drag(nodes, dt);
  }
  for(Node n : nodes) {
    n.lastPos = n.pos;
    n.vel = n.vel.plus(gravity.times(dt)).times(0.9999);
    Vec3 nextPos = n.pos.plus(n.vel.times(dt));
    float distToEarth = nextPos.minus(earthPos).length();
    if(distToEarth < earthRadiusC) {
      Vec3 norm = earthPos.minus(nextPos);
      norm.normalize();
      nextPos = nextPos.minus(norm.times(earthRadiusC - distToEarth));
    }
    n.pos = nextPos;
    n.neighbors = get_neighbors(n, dt);
  }

  // Constrain the distance between nodes to the link length
  for (int i = 0; i < numRelaxationSteps; i++) {
    update_positions();
    if(pinned) {
      for(int k = 0; k < numColumns; k++) {
          nodes.get(k*numRows).pos = new Vec3(basePos.x, basePos.y, basePos.z + (k * linkLength));
      }
    }
  }
  if (handleCollision) {
    resolve_collisions();
  }
  
  // Update the velocities (PBD)
  for(Node n : nodes) {
    n.vel = n.pos.minus(n.lastPos).times(1 / dt);
    if(n.pos.y > 11) {
      n.pos.y = 11;
      n.vel = n.vel.times(damping);
    }
  }
  
}

void update_positions() {
  for(int column = 0; column < numColumns; column++) {
    for(int row = 1; row < numRows; row++) {
      // correct vertically
      Vec3 delta = nodes.get(column * numRows + row).pos.minus(nodes.get(column * numRows + row - 1).pos);
      float delta_len = delta.length();
      float correction = delta_len - linkLength;
      Vec3 delta_normalized = delta.normalized();
      nodes.get(column * numRows + row).pos = nodes.get(column * numRows + row).pos.minus(delta_normalized.times(correction / 2));
      nodes.get(column * numRows + row-1).pos = nodes.get(column * numRows + row-1).pos.plus(delta_normalized.times(correction / 2));
      
      // correct horizontally
      if(column < numColumns - 1) {
        delta = nodes.get((column+1) * numRows + row).pos.minus(nodes.get(column * numRows + row).pos);
        delta_len = delta.length();
        correction = delta_len - linkLength;
        delta_normalized = delta.normalized();
        nodes.get((column+1) * numRows + row).pos = nodes.get((column+1) * numRows + row).pos.minus(delta_normalized.times(correction / 2));
        nodes.get(column * numRows + row).pos = nodes.get(column * numRows + row).pos.plus(delta_normalized.times(correction / 2));
        
        if(diagonalConstraints){
          delta = nodes.get((column+1) * numRows + row - 1).pos.minus(nodes.get(column * numRows + row).pos);
          delta_len = delta.length();
          correction = delta_len - linkLengthD;
          delta_normalized = delta.normalized();
          nodes.get((column+1) * numRows + row - 1).pos = nodes.get((column+1) * numRows + row - 1).pos.minus(delta_normalized.times(correction / 2));
          nodes.get(column * numRows + row).pos = nodes.get(column * numRows + row).pos.plus(delta_normalized.times(correction / 2));
        }
      }
      if(column > 0) {
        if(diagonalConstraints){
          delta = nodes.get(column * numRows + row).pos.minus(nodes.get((column - 1) * numRows + row - 1).pos);
          delta_len = delta.length();
          correction = delta_len - linkLengthD;
          delta_normalized = delta.normalized();
          nodes.get(column * numRows + row).pos = nodes.get(column * numRows + row).pos.minus(delta_normalized.times(correction / 2));
          nodes.get((column-1) * numRows + row-1).pos = nodes.get((column-1) * numRows + row-1).pos.plus(delta_normalized.times(correction / 2));
        }
      }
    }
  }
}

ArrayList<Integer> get_neighbors(Node p, float dt) {
  float maxNeighborDist = maxDist * dt;
  ArrayList<Integer> pNeighbors = new ArrayList<Integer>();
  for(int i = 0; i < nodes.size(); i ++) {
    float distToPSqr = dot(p.pos.minus(nodes.get(i).pos), (p.pos.minus(nodes.get(i).pos)));
    if(distToPSqr < maxNeighborDist*maxNeighborDist && distToPSqr > 0) {
      pNeighbors.add(i);
    }
  }
  return pNeighbors;
}

//simplified version of TenMinutePhysic's approach: https://www.youtube.com/watch?v=XY3dLpgOk4Q
void resolve_collisions() {
  for(Node n : nodes) {
    for (int i : n.neighbors) {
      float dist = n.pos.minus(nodes.get(i).pos).length();
      float basePosDist = n.basePos.minus(nodes.get(i).basePos).length();
      float minDist = min(basePosDist, thickness);
      if(dist < minDist) {
        Vec3 dir = n.pos.minus(nodes.get(i).pos).normalized();
        n.pos = n.pos.minus(dir.times((dist - minDist)/2)); //<>//
        nodes.get(i).pos = nodes.get(i).pos.plus(dir.times((dist - minDist)/2));
      }
    }
  }
}

void add_drag(ArrayList<Node> nodes, float dt) {
  for(int column = 0; column < numColumns; column++) {
    for(int row = 1; row < numRows; row++) {
      if(column < numColumns - 1) { 
        Vec3 drag1 = calculate_drag(
          nodes.get(column * numRows + row), 
          nodes.get(column * numRows + row - 1), 
          nodes.get((column+1) * numRows + row));
          
        Vec3 drag2 = calculate_drag(
          nodes.get(column * numRows + row), 
          nodes.get(column * numRows + row - 1), 
          nodes.get((column+1) * numRows + row-1));
          
          
        nodes.get(column * numRows + row).vel = nodes.get(column * numRows + row).vel.plus(drag1.times(dt));
        nodes.get(column * numRows + row).vel =  nodes.get(column * numRows + row).vel.plus(drag2.times(dt));
        nodes.get(column * numRows + row - 1).vel = nodes.get(column * numRows + row - 1).vel.plus(drag1.times(dt));
        nodes.get(column * numRows + row - 1).vel = nodes.get(column * numRows + row - 1).vel.plus(drag2.times(dt));
        nodes.get((column+1) * numRows + row).vel = nodes.get((column+1) * numRows + row).vel.plus(drag1.times(dt));
        nodes.get((column+1) * numRows + row-1).vel = nodes.get((column+1) * numRows + row-1).vel.plus(drag2.times(dt));
      }
    }
  }
}

Vec3 calculate_drag(Node n1, Node n2, Node n3) {
  float third = 1.0 / 3.0;
  Vec3 v = n1.vel.plus(n2.vel).plus(n3.vel).times(third).minus(wind);
  Vec3 nStar = cross(n2.pos.minus(n1.pos), n3.pos.minus(n1.pos));
  float denom = nStar.length() * 2;
  float num = dot(v, nStar) * v.length();
  Vec3 vecFactor = nStar.times(num/denom);
  float scalarFactor = -0.5 * third * airDensity * dragCoefficient;
  return vecFactor.times(scalarFactor);
}


float total_length_error(ArrayList<Node> nodes) {
  float length_error = 0.0;
  for(int i = 1; i < nodes.size(); i++) {
    length_error += nodes.get(i).pos.minus(nodes.get(i-1).pos).length() - linkLength; 
  }
  return length_error;
}

float total_energy(ArrayList<Node> nodes) {
    float kinetic_energy = 0;
    float potential_energy = 0;
    for(int i = 0; i < nodes.size(); i++) {
      kinetic_energy += 0.5 * nodes.get(i).vel.lengthSqr();
      potential_energy += gravity.length() * (10 - nodes.get(i).pos.y);
    }
    return kinetic_energy + potential_energy;
}

void initialize_scene() {
  camera = new Camera();
  camera.position = new PVector(90, 27, 101);
  camera.theta = 0.4249;
  camera.phi = -0.4362;
  nodes.clear();
  for(int i = 0; i < numNodes; i++) {
    float y_val = basePos.y - i%numRows * 0.0707;
    nodes.add(new Node(new Vec3(basePos.x + (i%numRows * 0.0707), y_val, basePos.z + (i/numRows * linkLength))));
  }
  for(Node n : nodes) {
    n.neighbors = get_neighbors(n, 1/fRate);
  }
  paused = true;
  pinned = true;
}

void initialize_scene_drop() {
  camera = new Camera();
  camera.position = new PVector(146, 22, -45);
  camera.theta = 2.3606;
  camera.phi = -0.3678;
  nodes.clear();
  for(int i = 0; i < numNodes; i++) {
    nodes.add(new Node(new Vec3(basePos.x + (i%numRows * linkLength), basePos.y - 3, basePos.z + (i/numRows * linkLength))));
  }
  for(Node n : nodes) {
    n.neighbors = get_neighbors(n, 1/fRate);
  }
  paused = true;
  pinned = false;
}

void setup() {
  size(1280, 720, P3D);
  frameRate(fRate);
  bg = loadImage("textures/space.jpg");
  earth = loadImage("textures/Globe.jpg");
  tex = loadImage("textures/texture.jpg");
  boxTex = loadImage("textures/wood.jpg");
  surface.setTitle("Cloth Simulation");
  initialize_scene();
}

void keyPressed()
{
  camera.HandleKeyPressed();
  if ( key == 'c' || key == 'C' ) {
    println(camera.position);
    println(camera.theta);
    println(camera.phi);
  }
  if ( key == 'm' || key == 'M' ) {
    meshMode = !meshMode;
    println("meshMode: " + meshMode);
  }
  if ( key == ' ' ) {
    paused = !paused;
  }
  if ( keyCode == CONTROL ) {
    stepMode = !stepMode;
    println("stepMode: " + stepMode);
  }
  if (  key == 'l' || key == 'L' ) {
    bg = loadImage("textures/BGTeal.jpg");
    earth = loadImage("textures/Orange.jpg");
    tex = loadImage("textures/check.jpg");
    boxTex = loadImage("textures/Green.jpg");
  }
  if (  key == 'k' || key == 'K' ) {
    bg = loadImage("textures/space.jpg");
    earth = loadImage("textures/Globe.jpg");
    tex = loadImage("textures/texture.jpg");
    boxTex = loadImage("textures/wood.jpg");
  }
  if (  key == '|') {
    wind = new Vec3(0,0,0);
    println("Wind:" + wind);
  }
  if (  key == ',') {
    wind.x += 1;
    println("Wind:" + wind);
  }
  if (  key == '.') {
    wind.y += 1;
    println("Wind:" + wind);
  }
  if (  key == '/') {
    wind.z += 1;
    println("Wind:" + wind);
  }
  if (  key == '<') {
     wind.x -= 1;
     println("Wind:" + wind);
  }
  if (  key == '>') {
     wind.y -= 1;
     println("Wind:" + wind);
  }
  if (  key == '?') {
     wind.z -= 1;
     println("Wind:" + wind);
  }
  if (  key == 'p' || key == 'P' ) {
     pinned = ! pinned;
  }
   if (  key == 'z' || key == 'Z' ) {
    directionalLight = !directionalLight;
    println("Directional lighting: " + directionalLight);
  }
  if (  key == 'v' || key == 'V' ) {
     handleDrag = ! handleDrag;
     println("Air drag: " + handleDrag);
  }
  if (  key == 'x' || key == 'X' ) {
     handleCollision = ! handleCollision;
     println("Collision handling: " + handleCollision);
  }
  if (  key == 'b' || key == 'B' ) {
     diagonalConstraints = ! diagonalConstraints;
     println("Diagonal constraints: " + diagonalConstraints);
  }
  if (  key == 'f' || key == 'F' ) {
    tex = loadImage("textures/check.jpg");
  }
  if (  key == 'g' || key == 'G' ) {
    tex = loadImage("textures/texture.jpg");
  }
  if (  key == '1') {
    initialize_scene();
  }
  if (  key == '2') {
    initialize_scene_drop();
  }
  if (  key == '3') {
     tex = loadImage("textures/pirate.png");
  }
  if (  key == '4') {
    earth = loadImage("textures/pirate.png");
  }
  if (  key == '5') {
    boxTex = loadImage("textures/pirate.png");
  }
  if (  key == '6') {
    bg = loadImage("textures/pirateBG.png");
  }
}

void keyReleased()
{
  camera.HandleKeyReleased();
}


void draw() {
  if(directionalLight){
    pointLight(255, 255, 200, 0.1 * sceneScale, 5.6 * sceneScale, 2.7 * sceneScale);
    pointLight(255, 255, 200, 9 * sceneScale, 2.7 * sceneScale, 10.1 * sceneScale);
  }
  ambientLight(100, 100, 100);
  camera.Update(1.0/frameRate);
  
  float dt = 1.0 / frameRate;
  if (!paused) {
    for (int i = 0; i < numSubsteps; i++) {
      time += dt / numSubsteps;
      update_physics(dt / numSubsteps);
    }
  }
  
  //background and scene objects
  background(bg);
  noStroke();
  PShape globe = createShape(SPHERE, earthRadius * sceneScale); 
  globe.setTexture(earth);
  
  pushMatrix();
  translate(earthPos.x * sceneScale, earthPos.y * sceneScale, earthPos.z * sceneScale);
  shape(globe);
  popMatrix();
  
  PShape box = createShape(BOX, 100); 
  box.setTexture(boxTex);
  pushMatrix();
  translate( basePos.x * sceneScale, 16.01 * sceneScale,  basePos.z * sceneScale );
  //fill(0,0,0);
  //box(100);
  shape(box);
  popMatrix();
  
  textureMode(NORMAL);   
  // setup to tecture cloth using multiple rows of triangle strips based off approach in this processing tutorial:
  // https://www.youtube.com/watch?v=FeXnJSCFj-Q
  for(int i = 0; i < (numColumns - 1); i++) {
    strokeWeight(0.2 * sceneScale);
    beginShape(TRIANGLE_STRIP);
    texture(tex);
    for(int j = 0; j < numRows; j++) {
      float tex_x_1 = 1.0 - (float) i / (numColumns - 1);
      float tex_x_2 = 1.0 - (float)(i + 1) / (numColumns - 1);
      float tex_y = 1.0 - (float)j / (numRows-1);
      int ind1 = i * numRows + j;
      int ind2 = (i + 1) * numRows + j;
      vertex(nodes.get(ind1).pos.x * sceneScale, nodes.get(ind1).pos.y * sceneScale, nodes.get(ind1).pos.z * sceneScale, tex_x_1, tex_y);
      vertex(nodes.get(ind2).pos.x * sceneScale, nodes.get(ind2).pos.y * sceneScale, nodes.get(ind2).pos.z * sceneScale, tex_x_2, tex_y);
      if(meshMode) {
        stroke(255);
        strokeWeight(0.2 * sceneScale);
        if(j > 0) {
          line((nodes.get(i * numRows + j - 1).pos.x * sceneScale), 
               (nodes.get(i * numRows + j - 1).pos.y * sceneScale),
               (nodes.get(i * numRows + j - 1).pos.z * sceneScale),
               (nodes.get(i * numRows + j).pos.x * sceneScale),
               (nodes.get(i * numRows + j).pos.y * sceneScale),
               (nodes.get(i * numRows + j).pos.z * sceneScale));
               
          if( i > 0) {
            line((nodes.get((i-1) * numRows + j - 1).pos.x * sceneScale), 
                 (nodes.get((i-1) * numRows + j - 1).pos.y * sceneScale),
                 (nodes.get((i-1) * numRows + j - 1).pos.z * sceneScale),
                 (nodes.get(i * numRows + j).pos.x * sceneScale),
                 (nodes.get(i * numRows + j).pos.y * sceneScale),
                 (nodes.get(i * numRows + j).pos.z * sceneScale));
          }
        }
        
        if( i > 0) {
          line((nodes.get((i-1) * numRows + j).pos.x * sceneScale), 
               (nodes.get((i-1) * numRows + j).pos.y * sceneScale),
               (nodes.get((i-1) * numRows + j).pos.z * sceneScale),
               (nodes.get(i * numRows + j).pos.x * sceneScale),
               (nodes.get(i * numRows + j).pos.y * sceneScale),
               (nodes.get(i * numRows + j).pos.z * sceneScale));
          if(i != numColumns - 1 && j != numRows - 1) {
            line((nodes.get((i-1) * numRows + j + 1).pos.x * sceneScale), 
                 (nodes.get((i-1) * numRows + j + 1).pos.y * sceneScale),
                 (nodes.get((i-1) * numRows + j + 1).pos.z * sceneScale),
                 (nodes.get(i * numRows + j).pos.x * sceneScale),
                 (nodes.get(i * numRows + j).pos.y * sceneScale),
                 (nodes.get(i * numRows + j).pos.z * sceneScale));
          }
        }
      }
    }
    endShape();
  }
  if (stepMode && !paused) {
    paused = true;
  }
}
