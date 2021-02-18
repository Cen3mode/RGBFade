#ifndef VECTOR_H
#define VECTOR_H

class Vector {
  private:

  public:

  double x = 0;
  double y = 0;
  double z = 0;

  Vector(double nx, double ny, double nz) {
    x = nx;
    y = ny;
    z = nz;
  }

  void add(Vector vector) {
    x += vector.x;
    y += vector.y;
    z += vector.z;
  }

  void sub(Vector vector) {
    x -= vector.x;
    y -= vector.y;
    z -= vector.z;
  }

  void mult(double factor) {
    x *= factor;
    y *= factor;
    z *= factor;
  }

  void set(double nx, double ny, double nz) {
    x = nx;
    y = ny;
    z = nz;
  }

  double mag() {
    return sqrt(pow(x, 2)+pow(y, 2)+pow(z, 2));
  }

  void norm() {
    double len = mag();
    x = x/len;
    y = y/len;
    z = z/len;
  }

};

#endif