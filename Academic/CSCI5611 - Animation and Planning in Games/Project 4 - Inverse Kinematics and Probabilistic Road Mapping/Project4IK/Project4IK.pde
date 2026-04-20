//All code in this file is either my own, or based off in class/instructor provided examples

//Classes
public class Link {
  Vec2 root;
  float len;
  float w;
  float angle;
  float baseAngleMin;
  float baseAngleMax;
  float angleMin;
  float angleMax;
  float rotationCap;
  String name;
  Link parent;
  
  public Link(Vec2 root, float angle, float len, float w, float angleMin, float angleMax, float rotationCap, String name, Link parent){
    this.root = root;
    this.angle = angle;
    this.len = len;
    this.w = w;
    this.baseAngleMin = angleMin;
    this.baseAngleMax = angleMax;
    this.angleMin = angleMin;
    this.angleMax = angleMax;
    this.rotationCap = rotationCap;
    this.name = name;
    this.parent = parent;
  }
  
  float getTotalAngle() {
    Link current = this;
    float totalAngle = current.angle;
    while (current.parent != null) {
      current = current.parent;
      totalAngle += current.angle;
    }
    return totalAngle;
  }
}

public class Obstacle {
  Vec2 root;
  float size;
  
  public Obstacle(Vec2 root, float size){
    this.root = root;
    this.size = size;
  }
}

public class Line {
  Vec2 p1;
  Vec2 p2;
  String name;
  public Line(Vec2 p1, Vec2 p2, String name){
    this.p1 = p1;
    this.p2 = p2;
    this.name = name;
  }
}

//Constants, shared variables
int w = 1280;
int h = 720;

boolean limits = true;
boolean paused = true;
boolean stepMode = false;
boolean oneArmMode = false;
int activeArm = 1;

//Shared static Root
Vec2 staticRoot = new Vec2(w/2,h/2 + 200.0);

Vec2 goalPos = new Vec2(w/2 + 1,h/2 - 200.0);
Vec2 goalMoveAdjustVec = new Vec2(0.0,0.0);
boolean goalSelected = false;
int goalSize = 50;

Vec2 obOnePos = new Vec2(w/2 + 200 + 1,h/2 - 200.0);
Vec2 obOneMoveAdjustVec = new Vec2(0.0,0.0);
boolean obOneSelected = false;
int obOneSize = 25;

Vec2 obTwoPos = new Vec2(w/2 - 200 + 1,h/2 - 200.0);
Vec2 obTwoMoveAdjustVec = new Vec2(0.0,0.0);
boolean obTwoSelected = false;
int obTwoSize = 25;

Obstacle o1 = new Obstacle(obOnePos, obOneSize);
Obstacle o2 = new Obstacle(obTwoPos, obTwoSize);

float armW = 25;
float armW2 = 15;
float armW3 = 20;

ArrayList<Obstacle> obstacles = new ArrayList<Obstacle>();
ArrayList<Link> links = new ArrayList<Link>();

//Root information
//Shared dynamic Roots
float spineSize = 60;
float spineDiag = 67.082; 
float spineAngle = 0.0;
float spineBaseMinAngle = -0.523;
float spineBaseMaxAngle = 0.523;
float spineCap = 0.01;

//Arm information
float ArmUpperLength = 120; 
float ArmUpperAngle = 0.0;
float ArmUpperCap = 0.03;

float ArmLowerLength = 100;
float ArmLowerAngle = 0.3;
float ArmLowerCap = 0.03;

float HandLength = 50;
float HandAngle = 0.3;
float HandCap = 0.06;

float uArmBaseMinAngle = -500;
float uArmBaseMaxAngle = 500;

float lArmBaseMinAngle = -2.8;
float lArmBaseMaxAngle = 2.8;

float handBaseMinAngle = -1.57;
float handBaseMaxAngle = 1.57;

Link spine1 = new Link(staticRoot, spineAngle, spineSize, spineSize, spineBaseMinAngle, spineBaseMaxAngle, spineCap, "spine1", null);
Link spine2 = new Link(staticRoot, spineAngle, spineSize, spineSize, spineBaseMinAngle, spineBaseMaxAngle, spineCap, "spine2", spine1);
Link spine3 = new Link(staticRoot, spineAngle, spineSize, spineSize, spineBaseMinAngle, spineBaseMaxAngle, spineCap, "spine3", spine2);

Link rArmUpper = new Link(staticRoot, ArmUpperAngle, ArmUpperLength, armW, uArmBaseMinAngle, uArmBaseMaxAngle, ArmUpperCap, "rArmUpper", spine3);
Link rArmLower = new Link(staticRoot, ArmLowerAngle, ArmLowerLength, armW2, lArmBaseMinAngle, lArmBaseMaxAngle, ArmLowerCap, "rArmLower", rArmUpper);
Link rHand = new Link(staticRoot, HandAngle, HandLength, armW3, handBaseMinAngle, handBaseMaxAngle, HandCap, "rHand", rArmLower);

Link lArmUpper = new Link(staticRoot, -ArmUpperAngle + 3.14, ArmUpperLength, armW, uArmBaseMinAngle, uArmBaseMaxAngle, ArmUpperCap, "lArmUpper", spine3);
Link lArmLower = new Link(staticRoot, -ArmLowerAngle, ArmLowerLength, armW2, lArmBaseMinAngle, lArmBaseMaxAngle, ArmLowerCap, "lArmLower", lArmUpper);
Link lHand = new Link(staticRoot, -HandAngle, HandLength, armW3, handBaseMinAngle, handBaseMaxAngle, HandCap, "lHand", lArmLower);

Vec2 rEndPoint, lEndPoint;

float Quadratic(float a, float b, float c) {
    float discriminant = (b * b) - (4 * a * c);
    //printf("discriminant: %f\n", discriminant);
    if (discriminant < 0) {
        return -999;
    }
    //if we are tangent
    else if (discriminant == 0) {
        return b / (2 * a);
    }
    else {
        float root;
        if (b >= 0) {
            root = (-b + sqrt(discriminant)) / 2;
        }
        else {
            root = (-b - sqrt(discriminant)) / 2;
        }
        float intersection_1 = root / a;
        float intersection_2 = c / root;
        float t_val = min(intersection_1, intersection_2);
        if (t_val > 0) {
            return t_val;
        }
        else {
            return -999;
        }
    }
}

boolean CircleLineTest(Obstacle o, Line l) {
    Vec2 p1ToCenter = l.p1.minus(o.root);
    Vec2 p2ToCenter = l.p2.minus(o.root);
    float sizeWithBuffer = o.size + 0.001;
    //if either endpoint is in circle, there is a collision
    if (dot(p1ToCenter, p1ToCenter) <= sizeWithBuffer * sizeWithBuffer || dot(p2ToCenter, p2ToCenter) <= sizeWithBuffer * sizeWithBuffer) {
      //println(l.name);
       return true;
    }
    else {
        Vec2 directionNorm = l.p1.minus(l.p2).normalized();
        float b = 2.0 * dot(directionNorm, p2ToCenter);
        float c = dot(p2ToCenter, p2ToCenter) - (sizeWithBuffer * sizeWithBuffer);
        //a is 1 since we normalize direction
        float tVal = Quadratic(1, b, c);
        float mag = l.p2.minus(l.p1).length();
        if (tVal >= 0 && tVal <= mag) {
          //println(l.name);
          return true;
        }
        return false;
    }
}

void solveForLink(Vec2 goal, Link l, Vec2 endPoint) {
  Vec2 startToGoal, startToEndEffector;
  float dotProd, angleDiff;
  
  startToGoal = goal.minus(l.root);
  startToEndEffector = endPoint.minus(l.root);
  dotProd = dot(startToGoal.normalized(),startToEndEffector.normalized());
  dotProd = clamp(dotProd,-1,1);
  angleDiff = acos(dotProd);
  float oldAngle = l.angle;
  int tries = 1000;
  do {
    l.angle = oldAngle;
    boolean positiveDiff = cross(startToGoal,startToEndEffector) < 0;
    if (positiveDiff){
      if(limits) {
        l.angle += angleDiff > l.rotationCap ? l.rotationCap : angleDiff;
        l.angle = l.angle > l.angleMax ? l.angleMax : l.angle;
      }
      else {
        l.angle += angleDiff;
      }
    }
    else{
      if(limits) {
        l.angle -= angleDiff > l.rotationCap ? l.rotationCap : angleDiff;
        l.angle = l.angle < l.angleMin ? l.angleMin : l.angle;
      }
      else {
        l.angle -= angleDiff;
      }
    }
     angleDiff *= 0.5;
    fk();
    tries--;
  } while(CheckCollisions() && tries > 0);  
}

void solve(){ 
  Vec2 goal = goalPos;
  if((!oneArmMode && goal.minus(lEndPoint).length() >= goal.minus(rEndPoint).length()) || oneArmMode && activeArm == 1) {
  solveForLink(goal, rHand, rEndPoint);
  solveForLink(goal, rArmLower, rEndPoint);
  solveForLink(goal, rArmUpper, rEndPoint);
  }
  if ((!oneArmMode && goal.minus(lEndPoint).length() < goal.minus(rEndPoint).length()) || oneArmMode && activeArm == 2) {
  solveForLink(goal, lHand, lEndPoint);
  solveForLink(goal, lArmLower, lEndPoint);
  solveForLink(goal, lArmUpper, lEndPoint);
  }
  if((!oneArmMode && goal.minus(lEndPoint).length() < goal.minus(rEndPoint).length()) || oneArmMode && activeArm == 2) {
    solveForLink(goal, spine3, lEndPoint);
    solveForLink(goal, spine2, lEndPoint);
    solveForLink(goal, spine1, lEndPoint);
  }
  if((!oneArmMode && goal.minus(lEndPoint).length() >= goal.minus(rEndPoint).length()) || oneArmMode && activeArm == 1) {
    solveForLink(goal, spine3, rEndPoint);
    solveForLink(goal, spine2, rEndPoint);
    solveForLink(goal, spine1, rEndPoint);
  }
  //println("lArmUAngle:",lArmUpper.angle,"lArmLAngle:",rArmLower.angle,"RHandAngle",rHand.angle);
  //println("spine2Angle:",spine2.angle,"spine1Angle:",spine1.angle);
  //println("RArmUAngle:",rArmUpper.angle,"RArmLAngle:",rArmLower.angle,"RHandAngle",rHand.angle);
}

boolean CheckCollisions(){
  ArrayList<Line> lines = generateLines();
  
  //println("RootX:", rArmUpper.root.x, "RootY:", rArmUpper.root.y, "spineAngle:");
  for(Line l : lines) {
    for (Obstacle o : obstacles) {
        if(CircleLineTest(o, l)) {
          return true;
        }
    }
  }
  return false;
}

boolean fk(){
  spine2.root = new Vec2(spine1.root.x + sin(spine1.angle) * spine1.len, spine1.root.y - cos(spine1.angle) * spine1.len);
  spine3.root = new Vec2(spine2.root.x + sin(spine2.getTotalAngle()) * spine2.len, spine2.root.y - cos(spine2.getTotalAngle()) * spine2.len);
  Vec2 spine3URCorner = new Vec2(spine3.root.x + cos(1.107 - spine3.getTotalAngle()) * spineDiag, spine3.root.y - sin(1.107 - spine3.getTotalAngle()) * spineDiag);
  Vec2 spine3ULCorner = new Vec2(spine3.root.x + cos(2.033 - spine3.getTotalAngle()) * spineDiag, spine3.root.y - sin(2.033 - spine3.getTotalAngle()) * spineDiag);
  
  rArmUpper.root = spine3URCorner;
  rArmLower.root = new Vec2(cos(rArmUpper.getTotalAngle())*rArmUpper.len,sin(rArmUpper.getTotalAngle())*rArmUpper.len).plus(rArmUpper.root);
  rHand.root = new Vec2(cos(rArmLower.getTotalAngle())*rArmLower.len,sin(rArmLower.getTotalAngle())*rArmLower.len).plus(rArmLower.root);
  rEndPoint = new Vec2(cos(rHand.getTotalAngle())*rHand.len,sin(rHand.getTotalAngle())*rHand.len).plus(rHand.root);
  
  lArmUpper.root = spine3ULCorner;
  lArmLower.root = new Vec2(cos(lArmUpper.getTotalAngle())*lArmUpper.len,sin(lArmUpper.getTotalAngle())*lArmUpper.len).plus(lArmUpper.root);
  lHand.root = new Vec2(cos(lArmLower.getTotalAngle())*lArmLower.len,sin(lArmLower.getTotalAngle())*lArmLower.len).plus(lArmLower.root);
  lEndPoint = new Vec2(cos(lHand.getTotalAngle())*lHand.len,sin(lHand.getTotalAngle())*lHand.len).plus(lHand.root);
  
  //println("RootX:", rArmUpper.root.x, "RootY:", rArmUpper.root.y, "spineAngle:");
  return false;
}

ArrayList<Line> generateLines(){
  ArrayList<Line> lines = new ArrayList<Line>();
  int i = 0;
  for(Link l : links) {
    ArrayList<Line> linkLines = generateLinesForLink(l, i <= 2);
    for(Line li : linkLines) {
      lines.add(li);
    }
    i++;
  }
  return lines;
}

ArrayList<Line> generateLinesForLink(Link l, boolean isSpine){
  ArrayList<Line> lines = new ArrayList<Line>();
  float diag = sqrt(l.len * l.len + (l.w/2) * (l.w/2));
  float ang = atan(l.len/(l.w/2));
  float invAng = 3.14 - ang;
  
  Vec2 linkURCorner = new Vec2(l.root.x + sin(l.getTotalAngle() + ang) * diag, l.root.y - cos(l.getTotalAngle() + ang) * diag);
  Vec2 linkULCorner = new Vec2(l.root.x - cos(l.getTotalAngle() + 1.57) * l.w/2, l.root.y - sin(l.getTotalAngle() + 1.57) * l.w/2);
  Vec2 linkBRCorner = new Vec2(l.root.x + sin(l.getTotalAngle() + invAng) * diag, l.root.y - cos(l.getTotalAngle() + invAng) * diag);
  Vec2 linkBLCorner = new Vec2(l.root.x + cos(l.getTotalAngle() + 1.57) * l.w/2, l.root.y + sin(l.getTotalAngle() + 1.57) * l.w/2);
  
  if(isSpine) {
    linkURCorner = new Vec2(l.root.x + cos(1.107 - l.getTotalAngle()) * diag, l.root.y - sin(1.107 - l.getTotalAngle()) * diag);
    linkULCorner = new Vec2(l.root.x + cos(2.033 - l.getTotalAngle()) * diag, l.root.y - sin(2.033 - l.getTotalAngle()) * diag);
    linkBRCorner = new Vec2(l.root.x + cos(l.getTotalAngle()) * l.w/2, l.root.y + sin(l.getTotalAngle()) * l.w/2);
    linkBLCorner = new Vec2(l.root.x - cos(l.getTotalAngle()) * l.w/2, l.root.y - sin(l.getTotalAngle()) * l.w/2);
  }
  
  lines.add(new Line(linkULCorner, linkURCorner, l.name + "-top"));
  lines.add(new Line(linkURCorner, linkBRCorner, l.name + "-right"));
  lines.add(new Line(linkBRCorner, linkBLCorner, l.name + "-bottom"));
  lines.add(new Line(linkBLCorner, linkULCorner, l.name + "-left"));
  
  return lines;
}

void keyPressed()
{
  if ( key == ' ' ) {
    paused = !paused;
  }
  
  if ( key == 'm' ) {
    println(mouseX, mouseY);
  }
  
  if ( key == 'l' ) {
    limits = !limits;
    println("Angle and Rotation speed limits: ", limits);
  }
  
  if ( key == '1' ) {
    oneArmMode = true;
    activeArm = 1;
  }
  
  if ( key == '2' ) {
    oneArmMode = true;
    activeArm = 2;
  }
  if ( key == '3' ) {
    oneArmMode = false;
  }
  if ( key == '.' ) {
    stepMode = !stepMode;
  }

}

void mousePressed()
{
  //println(mouseX, mouseY);
  Vec2 mousePos = new Vec2(mouseX, mouseY);
  if(mousePos.minus(goalPos).length() < goalSize) {
     goalMoveAdjustVec = goalPos.minus(mousePos);
     goalSelected = true;
  }
  else if(mousePos.minus(o1.root).length() < obOneSize) {
     obOneMoveAdjustVec = o1.root.minus(mousePos);
     obOneSelected = true;
  }
  else if(mousePos.minus(o2.root).length() < obTwoSize) {
     obTwoMoveAdjustVec = o2.root.minus(mousePos);
     obTwoSelected = true;
  }
}

void mouseDragged()
{
  //println(mouseX, mouseY);
  Vec2 mousePos = new Vec2(mouseX, mouseY);
  if(goalSelected) {
     goalPos = mousePos.plus(goalMoveAdjustVec);
  }
  if(obOneSelected) {
     o1.root = mousePos.plus(obOneMoveAdjustVec);
  }
  if(obTwoSelected) {
     o2.root = mousePos.plus(obTwoMoveAdjustVec);
  }
}

void mouseReleased() {
  goalSelected = false;
  obOneSelected = false;
  obTwoSelected = false;
}

void setup(){
  size(1280, 720);
  surface.setTitle("Inverse Kinematics simulation");
  obstacles.add(o1);
  obstacles.add(o2);
  links.add(spine1);
  links.add(spine2);
  links.add(spine3);
  links.add(rArmUpper);
  links.add(rArmLower);
  links.add(rHand);
  links.add(lArmUpper);
  links.add(lArmLower);
  links.add(lHand);
  frameRate(60);
  fk();
}

void draw(){
  if(!paused) {
  fk();
  solve();
  if(stepMode) paused = true;
  }
  
  background(250,250,250);
  
  fill(200,0,0);
  circle(goalPos.x, goalPos.y, goalSize);
  
  fill(0,0,200);
  circle(o1.root.x, o1.root.y, obOneSize * 2);
  circle(o2.root.x, o2.root.y, obTwoSize * 2);
  

  fill(200,150,120);
  pushMatrix();
  translate(spine1.root.x,spine1.root.y);
  rotate(spine1.angle);
  rect(-spine1.len/2, -spine1.len, spine1.len, spine1.len);
  popMatrix();
  
  pushMatrix();
  translate(spine2.root.x,spine2.root.y);
  rotate(spine2.getTotalAngle());
  rect(-spine2.len/2, -spine2.len, spine2.len, spine2.len);
  popMatrix();
  
  pushMatrix();
  translate(spine3.root.x,spine3.root.y);
  rotate(spine3.getTotalAngle());
  rect(-spine3.len/2, -spine3.len, spine3.len, spine3.len);
  popMatrix();
  
  pushMatrix();
  translate(rArmUpper.root.x,rArmUpper.root.y);
  rotate(rArmUpper.getTotalAngle());
  rect(0, -armW/2, rArmUpper.len, armW);
  popMatrix();
  
  
  pushMatrix();
  translate(rArmLower.root.x,rArmLower.root.y);
  rotate(rArmLower.getTotalAngle());
  rect(0, -armW2/2, rArmLower.len, armW2);
  popMatrix();
  
  pushMatrix();
  translate(rHand.root.x,rHand.root.y);
  rotate(rHand.getTotalAngle());
  rect(0, -armW3/2, rHand.len, armW3);
  popMatrix();
  
  pushMatrix();
  translate(lArmUpper.root.x,lArmUpper.root.y);
  rotate(lArmUpper.getTotalAngle());
  rect(0, -armW/2, lArmUpper.len, armW);
  popMatrix();
  
  
  pushMatrix();
  translate(lArmLower.root.x,lArmLower.root.y);
  rotate(lArmLower.getTotalAngle());
  rect(0, -armW2/2, lArmLower.len, armW2);
  popMatrix();
  
  pushMatrix();
  translate(lHand.root.x,lHand.root.y);
  rotate(lHand.getTotalAngle());
  rect(0, -armW3/2, lHand.len, armW3);
  popMatrix();
  
  //for(Line l : generateLines()) {
  //  line(l.p1.x, l.p1.y, l.p2.x, l.p2.y);
  //}
}



//-----------------
// Vector Library
//-----------------

//Vector Library
//CSCI 5611 Vector 2 Library [Example]
// Stephen J. Guy <sjguy@umn.edu>

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

float clamp(float f, float min, float max){
  if (f < min) return min;
  if (f > max) return max;
  return f;
}
