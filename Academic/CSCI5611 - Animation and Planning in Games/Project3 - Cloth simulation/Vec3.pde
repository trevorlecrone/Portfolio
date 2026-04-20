//---------------
//Vec 3 Library
//---------------

//Vector Library
//Adapted from Stephen J. Guy <sjguy@umn.edu> CSCI 5611 Vector 2 Library

public class Vec3 {
  public float x, y, z;

  public Vec3(float x, float y, float z) {
    this.x = x;
    this.y = y;
    this.z = z;
  }

  public String toString() {
    return "(" + x + "," + y + "," + z + ")";
  }

  public float length() {
    return sqrt(x * x + y * y + z * z);
  }

  public float lengthSqr() {
    return x * x + y * y + z * z;
  }

  public Vec3 plus(Vec3 rhs) {
    return new Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
  }

  public void add(Vec3 rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
  }

  public Vec3 minus(Vec3 rhs) {
    return new Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
  }

  public void subtract(Vec3 rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
  }

  public Vec3 times(float rhs) {
    return new Vec3(x * rhs, y * rhs, z * rhs);
  }

  public void mul(float rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
  }

  public void clampToLength(float maxL) {
    float magnitude = sqrt(x * x + y * y + z * z);
    if (magnitude > maxL) {
      x *= maxL / magnitude;
      y *= maxL / magnitude;
      z *= maxL / magnitude;
    }
  }

  public void setToLength(float newL) {
    float magnitude = sqrt(x * x + y * y + z * z);
    x *= newL / magnitude;
    y *= newL / magnitude;
    z *= newL / magnitude;
  }

  public void normalize() {
    float magnitude = sqrt(x * x + y * y + z * z);
    x /= magnitude;
    y /= magnitude;
    z /= magnitude;
  }

  public Vec3 normalized() {
    float magnitude = sqrt(x * x + y * y + z * z);
    return new Vec3(x / magnitude, y / magnitude, z / magnitude);
  }

  public float distanceTo(Vec3 rhs) {
    float dx = rhs.x - x;
    float dy = rhs.y - y;
    float dz = rhs.z - z;
    return sqrt(dx * dx + dy * dy + dz * dz);
  }
  
  public boolean equals(Vec3 rhs) {
    return x == rhs.x && y == rhs.y && z == rhs.z;
  }
}

Vec3 interpolate(Vec3 a, Vec3 b, float t) {
  return a.plus((b.minus(a)).times(t));
}

float interpolate(float a, float b, float t) {
  return a + ((b - a) * t);
}

float dot(Vec3 a, Vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 cross(Vec3 a, Vec3 b) {
  float new_x = (a.y * b.z) - (a.z * b.y);
  float new_y = (a.z * b.x) - (a.x * b.z);
  float new_z = (a.x * b.y) - (a.y * b.x);

  return new Vec3(new_x, new_y, new_z);
}

Vec3 projAB(Vec3 a, Vec3 b) {
  float dot_p = dot(a, b);
  float mag = b.length();
  return b.times(dot_p / mag);
}
