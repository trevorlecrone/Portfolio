#pragma once
#include "Vec.h"
#include "Ray.h"
#include "PLight.h"
#include "DLight.h"
#include "SLight.h"
#include "Material.h"
#include "FTriangle.h"
#include "NTriangle.h"
#include "Sphere.h"
#include "Camera.h"
#include "image.h"
#include <vector>

struct Scene {

public:
    Scene(): width(640), height(480), camera(Camera()), out_file("raytraced.bmp"), material(Material()), max_d(5){};

    void PrintMe() {
        
    }
    Material material;
    int width;
    int height;
    int max_d;
    Camera camera;
    Vec bg;
    Vec ambient;
    Vec* vertices;
    Vec* normals;
    std::vector<Surface*> objects;
    std::vector<Light*> lights;
    char out_file[1024];
};
