//With the exception of the createCylinder() function, All code in this file is either my own, 
//or based off in class/instructor provided examples

//createCylinder() is from this forumn post: https://forum.processing.org/two/discussion/26800/how-to-create-a-3d-cylinder-using-pshape-and-vertex-x-y-z.html

//A list of circle obstacles
static int numCircleObstacles = 50;
Vec2 circlePos[] = new Vec2[numCircleObstacles]; //Circle positions
float circleRad[] = new float[numCircleObstacles];  //Circle radii

//A box obstacle
static int numBoxObstacles = 10;
Vec2 topLefts[] = new Vec2[numBoxObstacles]; //Circle positions
float widths[] = new float[numBoxObstacles];  //Circle radii
float heights[] = new float[numBoxObstacles]; 

Vec2 startPos = new Vec2(100,500);
Vec2 goalPos = new Vec2(500,200);
float startingRadius = 100.0;
Vec2 targetPos = startPos;
Vec2 agentPos = startPos;
float agentSpeed = 200.0 / frameRate;
float agentRad = 20.0;
float agentTheta = 0.0;

ArrayList<Integer> passedNodes = new ArrayList<Integer>();

Camera camera = new Camera();

boolean paused = true;

boolean showLines = true;
boolean showAgentBounding = true;

boolean controlCamera = false;

int closestNode(Vec2 point){
  float min = 9999999.0;
  int minIndex = 0;
  for(int i = 0; i < numNodes; i++) {
    float dist = nodePos[i].minus(point).length();
    if(dist < min) {
     minIndex = i;
     min = dist;
    }
  }
  return minIndex;
}

int bestStartNode(Vec2 point){
  float min = 9999999.0;
  int bestNodeIndex = 0;
  float radius = startingRadius;
  ArrayList<Integer> indices = new ArrayList<Integer>();
  while(indices.size() == 0) {
    for(int i = 0; i < numNodes; i++) {
      float dist = nodePos[i].minus(point).length();
      if(dist < radius) {
       indices.add(i);
      }
    }
    radius += 10;
  }
  int goal = closestNode(goalPos);
  for(int index : indices) {
    float dist = runBFS(index, goal);
    if (dist < min) {
      min = dist;
      bestNodeIndex = index;
    }
  }
  return bestNodeIndex;
}


void placeRandomObstacles(){
  //Initial obstacle position
  for (int i = 0; i < numBoxObstacles; i++){
    boolean boxPlaced = false;
    do {
      Vec2 topLeft = new Vec2(random(50,950),random(50,700));
      float h = (10+40*pow(random(1),3));
      float w = (10+40*pow(random(1),3));
      boolean hit = boxBoxListIntersect(topLefts, widths, heights, topLeft, w, h, i);
      if(!hit) {
        topLefts[i] = topLeft;
        widths[i] = w;
        heights[i] = h;
        boxPlaced = true;
      }
    } while(!boxPlaced);
  }
  for (int i = 0; i < numCircleObstacles; i++){
    boolean circlePlaced = false;
    do {
      Vec2 center = new Vec2(random(50,950),random(50,700));
      float radius = (10+40*pow(random(1),3));
      hitInfo h = circleCircleListIntersect(circlePos, circleRad, center, radius, i);
      boolean boxHit = circleBoxListIntersect(topLefts, widths, heights, center, radius);
      if(!h.hit && !boxHit) {
        circlePos[i] = center;
        circleRad[i] = radius;
        circlePlaced = true;
      }
    } while(!circlePlaced);
  }
}



/////////
// Intersection Tests
/////////

//Returns true if the point is inside a box
boolean pointInBox(Vec2 boxTopLeft, float boxW, float boxH, Vec2 pointPos){
  float top = boxTopLeft.y - agentRad;
  float bottom = boxTopLeft.y + boxH + agentRad;
  float left = boxTopLeft.x - agentRad;
  float right = boxTopLeft.x + boxW + agentRad;
  if(pointPos.x >= left && pointPos.x <= right && pointPos.y <= bottom && pointPos.y >= top) {
    return true;
  }
  return false;
}

//Returns true if the point is inside a circle
boolean pointInCircle(Vec2 center, float r, Vec2 pointPos){
  float dist = pointPos.distanceTo(center);
  if (dist < r+2+agentRad){ //small safety factor
    return true;
  }
  return false;
}

//Returns true if the point is inside a list of circle
boolean pointInCircleList(Vec2[] centers, float[] radii, Vec2 pointPos){
  for (int i = 0; i < numCircleObstacles; i++){
    Vec2 center =  centers[i];
    float r = radii[i];
    if (pointInCircle(center,r,pointPos)){
      return true;
    }
  }
  return false;
}

boolean pointInBoxList(Vec2[] boxTopLefts, float[] boxWs, float[] boxHs, Vec2 pointPos){
  for (int i = 0; i < numBoxObstacles; i++){
    Vec2 tl =  boxTopLefts[i];
    float w = boxWs[i];
    float h = boxHs[i];
    if (pointInBox(tl, w, h, pointPos)){
      return true;
    }
  }
  return false;
}

class hitInfo{
  public boolean hit = false;
  public float t = 9999999;
}

hitInfo rayBoxIntersect(Vec2 boxTopLeft, float boxW, float boxH, Vec2 ray_start, Vec2 ray_dir, float max_t){
  hitInfo hit = new hitInfo();
  hit.hit = true;
    
  float t_left_x, t_right_x, t_top_y, t_bot_y;
  t_left_x = (boxTopLeft.x - ray_start.x)/ray_dir.x - agentRad;
  t_right_x = (boxTopLeft.x + boxW - ray_start.x)/ray_dir.x + agentRad;
  t_top_y = (boxTopLeft.y - ray_start.y)/ray_dir.y - agentRad;
  t_bot_y = (boxTopLeft.y + boxH - ray_start.y)/ray_dir.y + agentRad;
  
  float t_max_x = max(t_left_x,t_right_x);
  float t_max_y = max(t_top_y,t_bot_y);
  float t_max = min(t_max_x,t_max_y); //When the ray exists the box
  
  float t_min_x = min(t_left_x,t_right_x);
  float t_min_y = min(t_top_y,t_bot_y);
  float t_min = max(t_min_x,t_min_y); //When the ray enters the box
  
  
  //The the box is behind the ray (negative t)
  if (t_max < 0){
    hit.hit = false;
    hit.t = t_max;
    return hit;
  }
  
  //The ray never hits the box
  if (t_min > t_max){
    hit.hit = false;
  }
  
  //The ray hits, but further out than max_t
  if (t_min > max_t){
    hit.hit = false;
  }
  
  hit.t = t_min;
  return hit;
}

hitInfo rayBoxListIntersect(Vec2[] boxTopLefts, float[] boxWs, float[] boxHs, Vec2 ray_start, Vec2 ray_dir, float max_t){
  hitInfo hit = new hitInfo();
  hit.t = max_t;
  for (int i = 0; i < numBoxObstacles; i++){
    Vec2 boxTopLeft = boxTopLefts[i];
    float w = boxWs[i];
    float h = boxHs[i];
    
    hitInfo boxHit = rayBoxIntersect(boxTopLeft, w, h, ray_start, ray_dir, hit.t);
    if (boxHit.t > 0 && boxHit.t < hit.t){
      hit.hit = true;
      hit.t = boxHit.t;
    }
    else if (boxHit.hit && boxHit.t < 0){
      hit.hit = true;
      hit.t = -1;
    }
  }
  return hit;
}

hitInfo rayCircleIntersect(Vec2 center, float r, Vec2 l_start, Vec2 l_dir, float max_t){
  hitInfo hit = new hitInfo();
  
  //Step 2: Compute W - a displacement vector pointing from the start of the line segment to the center of the circle
    Vec2 toCircle = center.minus(l_start);
    
    //Step 3: Solve quadratic equation for intersection point (in terms of l_dir and toCircle)
    float a = 1;  //Length of l_dir (we normalized it)
    float b = -2*dot(l_dir,toCircle); //-2*dot(l_dir,toCircle)
    float c = toCircle.lengthSqr() - (r+strokeWidth)*(r+strokeWidth); //different of squared distances
    
    float d = b*b - 4*a*c; //discriminant 
    
    if (d >=0 ){ 
      //If d is positive we know the line is colliding, but we need to check if the collision line within the line segment
      //  ... this means t will be between 0 and the length of the line segment
      float t1 = (-b - sqrt(d))/(2*a); //Optimization: we only need the first collision
      float t2 = (-b + sqrt(d))/(2*a); //Optimization: we only need the first collision
      //println(hit.t,t1,t2);
      if (t1 > 0 && t1 < max_t){
        hit.hit = true;
        hit.t = t1;
      }
      else if (t1 < 0 && t2 > 0){
        hit.hit = true;
        hit.t = -1;
      }
      
    }
    
  return hit;
}

hitInfo rayCircleListIntersect(Vec2[] centers, float[] radii, Vec2 l_start, Vec2 l_dir, float max_t){
  hitInfo hit = new hitInfo();
  hit.t = max_t;
  for (int i = 0; i < numCircleObstacles; i++){
    Vec2 center = centers[i];
    float r = radii[i] + agentRad;
    
    hitInfo circleHit = rayCircleIntersect(center, r, l_start, l_dir, hit.t);
    if (circleHit.t > 0 && circleHit.t < hit.t){
      hit.hit = true;
      hit.t = circleHit.t;
    }
    else if (circleHit.hit && circleHit.t < 0){
      hit.hit = true;
      hit.t = -1;
    }
  }
  return hit;
}

hitInfo circleCircleIntersect(Vec2 c1, float r1, Vec2 c2, float r2){
  hitInfo hit = new hitInfo();
  if (c1.minus(c2).length() <= r1 + r2) {
    hit.hit = true;
    hit.t = c1.minus(c2).length();
  }
  else {
    hit.hit = false;
    hit.t = -1;
  }
  return hit;
}

hitInfo circleCircleListIntersect(Vec2[] centers, float[] radii, Vec2 targetC, float targetR, int numSetCircles){
  hitInfo hit = new hitInfo();
  hit.hit = false;
  for (int i = 0; i < numSetCircles; i++){
    Vec2 center = centers[i];
    float r = radii[i];
    
    hitInfo circleHit = circleCircleIntersect(center, r, targetC, targetR);
    if (circleHit.hit){
      hit.hit = true;
      hit.t = circleHit.t;
      return hit;
    }
    else{
      hit.hit = false;
      hit.t = -1;
    }
  }
  return hit;
}

hitInfo circleBoxIntersect(Vec2 targetC, float targetR, Vec2 boxTopLeft, float boxW, float boxH) {
    hitInfo hit = new hitInfo();
    float boxXMin = (boxTopLeft.x);
    float boxXMax = (boxTopLeft.x + boxW);
    float boxYMin = (boxTopLeft.y);
    float boxYMax = (boxTopLeft.y + boxH);
    float closestX = targetC.x <= boxXMin ? boxXMin : targetC.x >= boxXMax ? boxXMax : targetC.x;
    float closestY = targetC.y <= boxYMin ? boxYMin : targetC.y >= boxYMax ? boxYMax : targetC.y;
    Vec2 vectorToClosest = new Vec2(closestX, closestY).minus(targetC);
    hit.hit = (dot(vectorToClosest, vectorToClosest) <= targetR * targetR);
    return hit;
}

boolean boxBoxIntersect(Vec2 b1TopLeft, float b1W, float b1H, Vec2 b2TopLeft, float b2W, float b2H) {
    if (abs((b1TopLeft.x + (b1W / 2)) - (b2TopLeft.x + (b2W / 2))) > ((b1W + b2W) / 2)) {
        return false;
    }
    if (abs((b1TopLeft.y + (b1H / 2)) - (b2TopLeft.y + (b2H / 2))) > ((b1H + b2H) / 2)) {
        return false;
    }
    return true;
}

boolean boxBoxListIntersect(Vec2[] topLefts, float[] widths, float[] heights, Vec2 targetTL, float targetW, float targetH, int numSetBoxes){
  for (int i = 0; i < numSetBoxes; i++){
    Vec2 tl = topLefts[i];
    float w = widths[i];
    float h = heights[i];
    
    boolean hit = boxBoxIntersect(tl, w, h, targetTL, targetW, targetH);
    if (hit){
      return hit;
    }
  }
  return false;
}

boolean circleBoxListIntersect(Vec2[] topLefts, float[] widths, float[] heights, Vec2 targetC, float targetR){
  for (int i = 0; i < numBoxObstacles; i++){
    Vec2 tl = topLefts[i];
    float w = widths[i];
    float h = heights[i];
    
    hitInfo hit = circleBoxIntersect(targetC, targetR, tl, w, h);
    if (hit.hit){
      return hit.hit;
    }
  }
  return false;
}


/////////////////////////////////
// A Probabilistic Roadmap (PRM)
////////////////////////////////

static int numNodes = 150;

//The optimal path found along the PRM
ArrayList<Integer> path = new ArrayList();
int startNode, goalNode; //The actual node the PRM tries to connect do

//Represent our graph structure as 3 lists
ArrayList<Integer>[] neighbors = new ArrayList[numNodes];  //A list of neighbors can can be reached from a given node
Boolean[] visited = new Boolean[numNodes]; //A list which store if a given node has been visited
int[] parent = new int[numNodes]; //A list which stores the best previous node on the optimal path to reach this node

//The PRM uses the above graph, along with a list of node positions
Vec2[] nodePos = new Vec2[numNodes];

//Generate non-colliding PRM nodes
void generateRandomNodes(Vec2[] circleCenters, float[] circleRadii, Vec2[] boxTopLefts, float[] boxWs, float[] boxHs){
  for (int i = 0; i < numNodes; i++){
    Vec2 randPos = new Vec2(random(width),random(height));
    boolean insideAnyCircle = pointInCircleList(circleCenters, circleRadii, randPos);
    boolean insideAnyBox = pointInBoxList(topLefts, widths, heights, randPos);
    while (insideAnyCircle || insideAnyBox){
      randPos = new Vec2(random(width),random(height));
      insideAnyCircle = pointInCircleList(circleCenters,circleRadii,randPos);
      insideAnyBox = pointInBoxList(boxTopLefts, boxWs, boxHs, randPos);
    }
    nodePos[i] = randPos;
  }
}


//Set which nodes are connected to which neighbors based on PRM rules
void connectNeighbors(){
  for (int i = 0; i < numNodes; i++){
    neighbors[i] = new ArrayList<Integer>();  //Clear neighbors list
    for (int j = 0; j < numNodes; j++){
      if (i == j) continue; //don't connect to myself 
      Vec2 dir = nodePos[j].minus(nodePos[i]).normalized();
      float distBetween = nodePos[i].distanceTo(nodePos[j]);
      hitInfo circleListCheck = rayCircleListIntersect(circlePos, circleRad, nodePos[i], dir, distBetween);
      hitInfo boxListCheck = rayBoxListIntersect(topLefts, widths, heights, nodePos[i], dir, distBetween);
      if (!circleListCheck.hit && !boxListCheck.hit && distBetween <= 250.0){
        neighbors[i].add(j);
      }
    }
  }
}

//Build the PRM
// 1. Generate collision-free nodes
// 2. Connect mutually visible nodes as graph neighbors
void buildPRM(Vec2[] circleCenters, float[] circleRadii, Vec2[] boxTopLefts, float[] boxWs, float[] boxHs){
  generateRandomNodes(circleCenters, circleRadii, boxTopLefts, boxWs, boxHs);
  connectNeighbors();
}

//BFS
float runBFS(int startID, int goalID){
  startNode = startID;
  goalNode = goalID;
  boolean found = false;
  ArrayList<Integer> fringe = new ArrayList();  //Make a new, empty fringe
  float dist = 0.0;
  path = new ArrayList(); //Reset path
  for (int i = 0; i < numNodes; i++) { //Clear visit tags and parent pointers
    visited[i] = false;
    parent[i] = -1; //No parent yet
  }

  println("\nBeginning Search");
  
  visited[startID] = true;
  fringe.add(startID);
  println("Adding node", startID, "(start) to the fringe.");
  println(" Current Fring: ", fringe);
  
  while (fringe.size() > 0){
    int currentNode = fringe.get(0);
    fringe.remove(0);
    if (currentNode == goalID){
      println("Goal found!");
      found = true;
      break;
    }
    for (int i = 0; i < neighbors[currentNode].size(); i++){
      int neighborNode = neighbors[currentNode].get(i);
      if (!visited[neighborNode]){
        visited[neighborNode] = true;
        parent[neighborNode] = currentNode;
        fringe.add(neighborNode);
        println("Added node", neighborNode, "to the fringe.");
        println(" Current Fringe: ", fringe);
      }
    } 
  }
  
  print("\nReverse path: ");
  int prevNode = parent[goalID];
  path.add(0,goalID);
  print(goalID, " ");
  while (prevNode >= 0){
    print(prevNode," ");
    path.add(0,prevNode);
    prevNode = parent[prevNode];
  }
  print("\n");
  for (int i = 0; i < path.size() -1; i++) {
    dist += nodePos[path.get(i)].minus(nodePos[path.get(i+1)]).length();
  }
  if(found) {
    return dist;
  }
  return 9999999.0;
}

Vec2 getAgentDir() {
 Vec2 tip = new Vec2(agentPos.x + cos(1.57 + agentTheta) * agentRad,  agentPos.y + sin(1.57 + agentTheta) * agentRad);
 Vec2 dir = tip.minus(agentPos).normalized();
 return dir;
}

void moveAgent(){
  Vec2 goal = nodePos[goalNode];
  if(agentPos != goal) {
    float angle = cross(getAgentDir(), targetPos.minus(agentPos).normalized());
    if(abs(angle) > 0.1) {
      if(angle > 0) {
        agentTheta -= 0.03;
        return;
      }
      agentTheta += 0.03;
        return;
    }
    if(targetPos != goal) { //<>//
      boolean noCircleCollisions = !rayCircleListIntersect(circlePos, circleRad, agentPos, goalPos.minus(agentPos).normalized(), goalPos.minus(agentPos).length()).hit;
      boolean noBoxCollisions = !rayBoxListIntersect(topLefts, widths, heights, agentPos, goalPos.minus(agentPos).normalized(), goalPos.minus(agentPos).length()).hit;
      if(noCircleCollisions && noBoxCollisions) {
        targetPos = goal;
        agentPos = targetPos.minus(agentPos).length() < agentSpeed ? targetPos : agentPos.plus(targetPos.minus(agentPos).normalized().times(agentSpeed));
        return;
      }
      if(path.size() > 1) {
        boolean canSkip = true;
        while(passedNodes.size() + 1 < path.size() && canSkip) {
          Vec2 nextNode = nodePos[path.get(passedNodes.size() + 1)]; //<>//
          noCircleCollisions = !rayCircleListIntersect(circlePos, circleRad, agentPos, nextNode.minus(agentPos).normalized(), nextNode.minus(agentPos).length()).hit;
          noBoxCollisions = !rayBoxListIntersect(topLefts, widths, heights, agentPos, nextNode.minus(agentPos).normalized(), nextNode.minus(agentPos).length()).hit;
          if(noCircleCollisions && noBoxCollisions) {
            targetPos = nextNode;
            passedNodes.add(path.get(passedNodes.size()));
          }
          else {
            canSkip = false;
          }
        }
        if(targetPos.minus(agentPos).length() < agentSpeed) {
          agentPos = targetPos;
          passedNodes.add(path.get(passedNodes.size() - 1));
          return;
        }
        agentPos = agentPos.plus(targetPos.minus(agentPos).normalized().times(agentSpeed));
        return;
      }
    }
    else {
       agentPos = targetPos.minus(agentPos).length() < agentSpeed ? targetPos : agentPos.plus(targetPos.minus(agentPos).normalized().times(agentSpeed));
       return;
    }
  }
}


int strokeWidth = 2;
void setup(){
  size(1024,768, P3D);
  placeRandomObstacles();
  buildPRM(circlePos, circleRad, topLefts, widths, heights);
  runBFS(bestStartNode(startPos),closestNode(goalPos));
  agentPos = nodePos[startNode];
  camera = new Camera();
  camera.position = new PVector( 539, 991, 900 );
  camera.theta = 0.0314;
  camera.phi = 0.527;
}

void draw(){
  //println("FrameRate:",frameRate);
  background(180,180,200); //blueish background
  stroke(0,0,0);
  fill(255,255,255);
  directionalLight(180, 180, 180, -0.5, -0.5, -0.5);
  directionalLight(180, 180, 180, 0.5, -0.5, -0.5);
  ambientLight(20, 20, 20);
  
  camera.Update(1.0/frameRate);
  
  fill(130);
  pushMatrix();
  translate(512, 384, -612.1);
  PShape ground = createShape(BOX, 1223);
  PImage tex = loadImage("textures/brick.bmp");
  ground.setTexture(tex);
  shape(ground);
  popMatrix();
    
  
  noStroke();
  //Draw the circle obstacles
  for (int i = 0; i < numCircleObstacles; i++){
    Vec2 c = circlePos[i];
    float r = circleRad[i];
    fill(c.x / 5, c.y / 4, r * 5);
    //PShape cylinder = createCylinder(10, r, 20);
    //shape(cylinder, c.x, c.y);
    //circle(c.x,c.y,r*2);
    pushMatrix();
    translate(c.x, c.y);
    sphere(r);
    popMatrix();
  }
  
  fill(100,300,100);
  for (int i = 0; i < numBoxObstacles; i++){
    Vec2 tl = topLefts[i];
    float w = widths[i];
    float h = heights[i];
    fill(tl.x/10 + tl.y/5,w * 6, h * 4);
    pushMatrix();
    translate(tl.x + w/2, tl.y + h/2);
    box(w, h, 20);
    popMatrix();
  }
  fill(250,200,200);
  //rect(boxTopLeft.x, boxTopLeft.y, boxW, boxH);
  
  if(showLines) {
    //Draw PRM Nodes
    fill(0);
    for (int i = 0; i < numNodes; i++){
      circle(nodePos[i].x,nodePos[i].y,5);
    }
    
    //Draw graph
    stroke(100,100,100);
    strokeWeight(1);
    for (int i = 0; i < numNodes; i++){
      for (int j : neighbors[i]){
        line(nodePos[i].x,nodePos[i].y,nodePos[j].x,nodePos[j].y);
      }
    }
  }
  
  //Draw Start and Goal
  noStroke();
  fill(250,250,50);
  PShape agent = createShape();
  agent.beginShape(TRIANGLES);
  agent.vertex(0.0, -agentRad, 0.0);
  agent.vertex(sin(0.35) * agentRad, cos(0.35) * agentRad, 0.0);
  agent.vertex(-sin(0.35) * agentRad, cos(0.35) * agentRad, 0.0);
  
  agent.vertex(0.0, -agentRad, 5.0);
  agent.vertex(sin(0.35) * agentRad, cos(0.35) * agentRad, 5.0);
  agent.vertex(-sin(0.35) * agentRad, cos(0.35) * agentRad, 5.0);
  
  agent.vertex(0.0, -agentRad, 0.0);
  agent.vertex(sin(0.35) * agentRad, cos(0.35) * agentRad, 0.0);
  agent.vertex(sin(0.35) * agentRad, cos(0.35) * agentRad, 5.0);
  
  agent.vertex(0.0, -agentRad, 0.0);
  agent.vertex(0.0, -agentRad, 5.0);
  agent.vertex(sin(0.35) * agentRad, cos(0.35) * agentRad, 5.0);
  
  agent.vertex(0.0,  -agentRad, 0.0);
  agent.vertex(-sin(0.35) * agentRad, cos(0.35) * agentRad, 0.0);
  agent.vertex(-sin(0.35) * agentRad, cos(0.35) * agentRad, 5.0);
  
  agent.vertex(0.0, -agentRad, 0.0);
  agent.vertex(0.0, -agentRad, 5.0);
  agent.vertex(- sin(0.35) * agentRad, cos(0.35) * agentRad, 5.0);
  
  agent.vertex(sin(0.35) * agentRad, cos(0.35) * agentRad, 0.0);
  agent.vertex(-sin(0.35) * agentRad, cos(0.35) * agentRad, 0.0);
  agent.vertex(-sin(0.35) * agentRad, cos(0.35) * agentRad, 5.0);
  
  agent.vertex(sin(0.35) * agentRad, cos(0.35) * agentRad, 0.0);
  agent.vertex(-sin(0.35) * agentRad, cos(0.35) * agentRad, 5.0);
  agent.vertex(sin(0.35) * agentRad, cos(0.35) * agentRad, 5.0);
  
  agent.endShape();
  pushMatrix();
  translate(agentPos.x,agentPos.y);
  rotate(agentTheta);
  shape(agent);
  if((showAgentBounding))
  {
    fill(255);
    circle(0.0,0.0,agentRad * 2);
  }
  popMatrix();
  //circle(startPos.x,startPos.y,20);
  fill(250,30,50);
  pushMatrix();
  translate(nodePos[goalNode].x,nodePos[goalNode].y);
  PShape goal = createCylinder(10, 10, 150.0);
  shape(goal);
  popMatrix();
  //circle(nodePos[goalNode].x,nodePos[goalNode].y,20);
  //circle(goalPos.x,goalPos.y,20);
  if(showLines) {
    //Draw Planned Path
    stroke(20,255,40);
    strokeWeight(5);
    for (int i = 0; i < path.size()-1; i++){
      int curNode = path.get(i);
      int nextNode = path.get(i+1);
      line(nodePos[curNode].x,nodePos[curNode].y,nodePos[nextNode].x,nodePos[nextNode].y);
    }
  }
  if(!paused) {
    moveAgent();
  }
   //println(frameRate);
  
}

void keyPressed(){
  //camera.HandleKeyPressed();
  if ( key == 'c' || key == 'C' ) {
    println(camera.position);
    println(camera.theta);
    println(camera.phi);
  }
  
  if ( key == 'x' || key == 'X' ) {
    controlCamera = !controlCamera;
  }
  if ( key == 'z' || key == 'Z' ) {
    showLines = !showLines;
    println("Showing Lines: ", showLines);
  }
  if ( key == 'b' || key == 'B' ) {
    showAgentBounding = !showAgentBounding;
    println("Showing Agent Bounding Circle: ", showAgentBounding);
  }
  
  if (key == 'r'){
    placeRandomObstacles();
    buildPRM(circlePos, circleRad, topLefts, widths, heights);
    runBFS(bestStartNode(startPos),closestNode(goalPos));
    passedNodes.clear();
    agentPos = nodePos[startNode];
    targetPos = agentPos;
    agentTheta = 0.0;
    paused = true;
    camera.position = new PVector( 539, 991, 900 );
    camera.theta = 0.0314;
    camera.phi = 0.527;
  }
  if (key == 'v') {
    camera.position = new PVector( 539, 991, 900 );
    camera.theta = 0.0314;
    camera.phi = 0.527;
  }
  if (key == ' '){
    paused = !paused;
  }
  if(controlCamera && key != 'r') {
    camera.HandleKeyPressed();
  }
  else{
    if (keyCode == RIGHT){
      int newIndex = (goalNode + 1) >= numNodes ? numNodes - 1 : goalNode + 1;
      goalNode = newIndex;
      goalPos = nodePos[goalNode];
      runBFS(bestStartNode(startPos),closestNode(goalPos));
      agentPos = nodePos[startNode];
      targetPos = agentPos;
      passedNodes.clear();
      agentTheta = 0.0;
      paused = true;
    }
    if (keyCode == LEFT){
      int newIndex = (goalNode - 1) <= 0 ? 0 : goalNode - 1;
      goalNode = newIndex;
      goalPos = nodePos[goalNode];
      runBFS(bestStartNode(startPos),closestNode(goalPos));
      agentPos = nodePos[startNode];
      targetPos = agentPos;
      passedNodes.clear();
      agentTheta = 0.0;
      paused = true;
    }
    if (keyCode == UP){
      int newIndex = (goalNode + 10) >= numNodes ? numNodes - 1 : goalNode + 10;
      goalNode = newIndex;
      goalPos = nodePos[goalNode];
      runBFS(bestStartNode(startPos),closestNode(goalPos));
      agentPos = nodePos[startNode];
      targetPos = agentPos;
      passedNodes.clear();
      agentTheta = 0.0;
      paused = true;
    }
    if (keyCode == DOWN){
      int newIndex = (goalNode - 10) <= 0 ? 0 : goalNode - 10;
      goalNode = newIndex;
      goalPos = nodePos[goalNode];
      runBFS(bestStartNode(startPos),closestNode(goalPos));
      agentPos = nodePos[startNode];
      targetPos = agentPos;
      passedNodes.clear();
      agentTheta = 0.0;
      paused = true;
    }
  }
}



//void mousePressed(){
//  goalPos = new Vec2(mouseX, mouseY);
//  println("New Goal is",goalPos.x, goalPos.y);
//  runBFS(bestStartNode(startPos),closestNode(goalPos));
//  agentPos = nodePos[startNode];
//  passedNodes.clear();
//  paused = true;
//}

void keyReleased()
{
  camera.HandleKeyReleased();
}


PShape createCylinder(int sides, float r, float h) {

  PShape cylinder = createShape(GROUP);

  float angle = 360 / sides;
  float halfHeight = h / 2;

  // draw top of the tube
  PShape top = createShape();
  top.beginShape();
  for (int i = 0; i < sides; i++) {
    float x = cos( radians( i * angle ) ) * r;
    float y = sin( radians( i * angle ) ) * r;
    top.vertex( x, y, -halfHeight);
  }
  top.endShape(CLOSE);
  cylinder.addChild(top);

  // draw bottom of the tube
  PShape bottom = createShape();
  bottom.beginShape();
  for (int i = 0; i < sides; i++) {
    float x = cos( radians( i * angle ) ) * r;
    float y = sin( radians( i * angle ) ) * r;
    bottom.vertex( x, y, halfHeight);
  }
  bottom.endShape(CLOSE);
  cylinder.addChild(bottom);

  // draw sides
  PShape middle = createShape();
  middle.beginShape(TRIANGLE_STRIP);
  for (int i = 0; i < sides + 1; i++) {
    float x = cos( radians( i * angle ) ) * r;
    float y = sin( radians( i * angle ) ) * r;
    middle.vertex( x, y, halfHeight);
    middle.vertex( x, y, -halfHeight);
  }
  middle.endShape(CLOSE);
  cylinder.addChild(middle);

  return cylinder;
}

/////////////////////////////////
// Vec2 Library
////////////////////////////////

public class Vec2 {
  public float x, y;
  
  public Vec2(float x, float y){
    this.x = x;
    this.y = y;
  }
  
  public String toString(){
    return "(" + x+ "," + y +")";
  }
  
  public float length(){
    return sqrt(x*x+y*y);
  }
  
  public float lengthSqr(){
    return x*x+y*y;
  }
  
  public Vec2 plus(Vec2 rhs){
    return new Vec2(x+rhs.x, y+rhs.y);
  }
  
  public void add(Vec2 rhs){
    x += rhs.x;
    y += rhs.y;
  }
  
  public Vec2 minus(Vec2 rhs){
    return new Vec2(x-rhs.x, y-rhs.y);
  }
  
  public void subtract(Vec2 rhs){
    x -= rhs.x;
    y -= rhs.y;
  }
  
  public Vec2 times(float rhs){
    return new Vec2(x*rhs, y*rhs);
  }
  
  public void mul(float rhs){
    x *= rhs;
    y *= rhs;
  }
  
  public void clampToLength(float maxL){
    float magnitude = sqrt(x*x + y*y);
    if (magnitude > maxL){
      x *= maxL/magnitude;
      y *= maxL/magnitude;
    }
  }
  
  public void setToLength(float newL){
    float magnitude = sqrt(x*x + y*y);
    x *= newL/magnitude;
    y *= newL/magnitude;
  }
  
  public void normalize(){
    float magnitude = sqrt(x*x + y*y);
    x /= magnitude;
    y /= magnitude;
  }
  
  public Vec2 normalized(){
    float magnitude = sqrt(x*x + y*y);
    return new Vec2(x/magnitude, y/magnitude);
  }
  
  public float distanceTo(Vec2 rhs){
    float dx = rhs.x - x;
    float dy = rhs.y - y;
    return sqrt(dx*dx + dy*dy);
  }
}

Vec2 interpolate(Vec2 a, Vec2 b, float t){
  return a.plus((b.minus(a)).times(t));
}

float interpolate(float a, float b, float t){
  return a + ((b-a)*t);
}

float dot(Vec2 a, Vec2 b){
  return a.x*b.x + a.y*b.y;
}

float cross(Vec2 a, Vec2 b){
  return a.x*b.y - a.y*b.x;
}

Vec2 projAB(Vec2 a, Vec2 b){
  return b.times(a.x*b.x + a.y*b.y);
}
