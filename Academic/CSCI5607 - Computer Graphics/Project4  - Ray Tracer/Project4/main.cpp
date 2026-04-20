//5607 Ray Tracer
//Trevor LeCrone 2017
//image, pixel, stb_image, and stb_image_write provided

#include "Scene.h"
#include "HitData.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <chrono>


#define STB_IMAGE_IMPLEMENTATION //only place once in one .cpp file
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION //only place once in one .cpp files
#include "stb_image_write.h"

//#define NTHREADS 1;
//#define NTHREADS 2;
//#define NTHREADS 4;
//#define NTHREADS 10;
#define NTHREADS 8;

using namespace std;

double Pi = acos(-1);

//compute focal plane
double compute_fp_z(double angle, int height, double cam_z) {
    double radian = angle * (Pi / 180);
    return (height / (2 * tan(radian))) + cam_z;
}

//compute angle fovx, assumes fgiven fovy half angle
double compute_fov_y(double angle, int height, int width) {
    double ar = (double)width / (double)height;
    double ha = atan(ar * tan(angle));
    return ha;
}

Pixel EvaluateRayTree(Scene scene, Ray ray, int depth, int max);

//debug version of color at
/*
Pixel DLModel(HitData h, FILE *f, int depth, int max) {
    Vec i_point = h.point;
    Vec normal = h.s_normal;
    Vec ambient_l = h.s.ambient;
    Vec mat_trans = h.mat.GetTransmissive();
    Vec mat_spec = h.mat.GetSpecular();
    Vec mat_color = h.mat.GetAmbient();
    Vec mat_diff = h.mat.GetDiffuse();
    Vec reflected;
    bool shadowed = false;
    double mat_phong = h.mat.GetPhong();
    double r = ((255 * ambient_l.X()) * mat_color.X());
    double g = ((255 * ambient_l.Y()) * mat_color.Y());
    double b = ((255 * ambient_l.Z()) * mat_color.Z());
    fprintf(f, "amb_r: %f, amb_g: %f, amb_b: %f\n", r, g, b);
    for (int l = 0; l < h.s.lights.size(); l++) {
        Vec l_dir = h.s.lights[l]->GetDirection(i_point).Normalize();
        double l_dis = h.s.lights[l]->GetDistance(i_point);
        Ray shadow_check = Ray(i_point, l_dir);
        for (int i = 0; i < h.s.objects.size(); i++) {
            shadowed = h.s.objects[i]->CheckShadow(shadow_check, 0.000000001, l_dis);
            if (shadowed) break;
        }
        if (!shadowed) {
            double angle_factor = normal.Normalize() * l_dir.Normalize();
            Vec l_intense = h.s.lights[l]->IntensityAt(i_point);
            Vec s_reflected = (l_dir.Reflect(normal.Normalize()));
            reflected = (h.ray.GetDir() * -1).Reflect(normal.Normalize());
            //fprintf(f, "l_dir : %f, %f, %f\n", l_dir.X(), l_dir.Y(), l_dir.Z());
            //fprintf(f, "s_normal : %f, %f, %f\n", normal.Normalize().X(), normal.Normalize().Y(), normal.Normalize().Z());
            fprintf(f, "reflected : %f, %f, %f\n", reflected.X(), reflected.Y(), reflected.Z());
            //fprintf(f, "angle_factor: %f\n", angle_factor);
            //fprintf(f, "intensity : %f, %f, %f\n", l_intense.X(), l_intense.Y(), l_intense.Z());
            
            if (angle_factor > 0) {
                r += (255 * (l_intense.X() * angle_factor * mat_diff.X()));
                g += (255 * (l_intense.Y() * angle_factor * mat_diff.Y()));
                b += (255 * (l_intense.Z() * angle_factor * mat_diff.Z()));
            }
            //fprintf(f, "diff_r: %f, diff_g: %f, diff_b: %f\n", r, g, b);
            double spec_angle = ((h.s.camera.position - i_point).Normalize() * s_reflected.Normalize());
            //if (spec_angle < 0) spec_angle = -spec_angle;
            //fprintf(f, "spec_angle %f\n", spec_angle);
            //fprintf(f, "pow(spec_angle, mat_phong) %f\n", pow(spec_angle, mat_phong));
            if (spec_angle > 0) {
                r += (255 * (l_intense.X() * pow(spec_angle, mat_phong) * mat_spec.X()));
                g += (255 * (l_intense.Y() * pow(spec_angle, mat_phong) * mat_spec.Y()));
                b += (255 * (l_intense.Z() * pow(spec_angle, mat_phong) * mat_spec.Z()));
            }
        }      
    }
    Pixel reflected_p = EvaluateRayTree(h.s, Ray(i_point + (reflected.Normalize() * 0.00001), reflected.Normalize()), f, (depth + 1), max);
    r += (reflected_p.r * mat_spec.X());
    g += (reflected_p.g * mat_spec.Y());
    b += (reflected_p.b * mat_spec.Z());
    int i_or_o = -1;
    Vec refracted_dir = h.mat.Refract(normal, h.ray.GetDir()).Normalize();
    fprintf(f, "refraction vector: [%f, %f, %f]\n", refracted_dir.X(), refracted_dir.Y(), refracted_dir.Z());
    Ray refracted = Ray(i_point + (refracted_dir * 0.0000001), refracted_dir);
    Pixel refracted_p = EvaluateRayTree(h.s, refracted, f, (depth + 1), max);
    r += mat_trans.X() * refracted_p.r;
    g += mat_trans.Y() * refracted_p.g;
    b += mat_trans.Z() * refracted_p.b;
    Pixel f_color;
    f_color.SetClamp(r, g, b);

    ///////DEBUG STATEMENTS

    fprintf(f, "r: %f, g: %f, b: %f\n", r, g, b);
    fprintf(f, "p_r: %d, p_g, %d, p_b %d \n\n", f_color.r, f_color.g, f_color.b);

    return f_color;
}
*/
Pixel LModel(HitData h, int depth, int max) {
    Vec i_point = h.point;
    Vec normal = h.s_normal;
    Vec ambient_l = h.s.ambient;
    Vec mat_trans = h.mat.GetTransmissive();
    Vec mat_spec = h.mat.GetSpecular();
    Vec mat_color = h.mat.GetAmbient();
    Vec mat_diff = h.mat.GetDiffuse();
    Vec reflected;
    bool shadowed = false;
    double mat_phong = h.mat.GetPhong();
    double r = ((255 * ambient_l.X()) * mat_color.X());
    double g = ((255 * ambient_l.Y()) * mat_color.Y());
    double b = ((255 * ambient_l.Z()) * mat_color.Z());
    for (int l = 0; l < h.s.lights.size(); l++) {
        Vec l_dir = h.s.lights[l]->GetDirection(i_point).Normalize();
        double l_dis = h.s.lights[l]->GetDistance(i_point);
        Ray shadow_check = Ray(i_point, l_dir);
        for (int i = 0; i < h.s.objects.size(); i++) {
            shadowed = h.s.objects[i]->CheckShadow(shadow_check, 0.000000001, l_dis);
            if (shadowed) break;
        }
        if (!shadowed) {
            double angle_factor = normal.Normalize() * l_dir.Normalize();
            Vec l_intense = h.s.lights[l]->IntensityAt(i_point);
            Vec s_reflected = (l_dir.Reflect(normal.Normalize()));
            reflected = (h.ray.GetDir() * -1).Reflect(normal.Normalize());
            if (angle_factor > 0) {
                r += (255 * (l_intense.X() * angle_factor * mat_diff.X()));
                g += (255 * (l_intense.Y() * angle_factor * mat_diff.Y()));
                b += (255 * (l_intense.Z() * angle_factor * mat_diff.Z()));
            }
            double spec_angle = ((h.s.camera.position - i_point).Normalize() * s_reflected.Normalize());
            if (spec_angle > 0) {
                r += (255 * (l_intense.X() * pow(spec_angle, mat_phong) * mat_spec.X()));
                g += (255 * (l_intense.Y() * pow(spec_angle, mat_phong) * mat_spec.Y()));
                b += (255 * (l_intense.Z() * pow(spec_angle, mat_phong) * mat_spec.Z()));
            }
        }
    }
    Pixel reflected_p = EvaluateRayTree(h.s, Ray(i_point + (reflected.Normalize() * 0.00001), reflected.Normalize()), (depth + 1), max);
    r += (reflected_p.r * mat_spec.X());
    g += (reflected_p.g * mat_spec.Y());
    b += (reflected_p.b * mat_spec.Z());
    int i_or_o = -1;
    Vec refracted_dir = h.mat.Refract(normal, h.ray.GetDir()).Normalize();
    Ray refracted = Ray(i_point + (refracted_dir * 0.0000001), refracted_dir);
    Pixel refracted_p = EvaluateRayTree(h.s, refracted, (depth + 1), max);
    r += mat_trans.X() * refracted_p.r;
    g += mat_trans.Y() * refracted_p.g;
    b += mat_trans.Z() * refracted_p.b;
    Pixel f_color;
    f_color.SetClamp(r, g, b);

    return f_color;
}

/*
Pixel DEvaluateRayTree(Scene scene, Ray ray, FILE *f, int x, int y, int depth, int max) {
    if (depth > max) {
        return Pixel(0, 0, 0, 1);
    }
    double nearest_t = 2000;
    Vec s_normal;
    Material s_mat;
    HitData h;
    for (int i = 0; i < scene.objects.size(); i++) {
        if (!(scene.objects[i]->CheckCollision(ray) == -999)) {
            double new_t = scene.objects[i]->CheckCollision(ray);
            if (new_t < nearest_t) {
                nearest_t = new_t;
                s_normal = scene.objects[i]->GetNormal((ray.GetOrigin() + (ray.GetDir() * nearest_t))).Normalize();
                s_mat = scene.objects[i]->GetMaterial();
            }
        }
    }
    if (nearest_t < 2000) {
        Vec i_point = (ray.GetOrigin() + (ray.GetDir() * nearest_t));
        h = HitData(s_mat, i_point, s_normal, scene, ray);
        ////DEBUG STATEMENTS

        fprintf(f, "direction vector: [%f, %f, %f]\n", ray.GetDir().X(), ray.GetDir().Y(), ray.GetDir().Z());
        fprintf(f, "x,y: %d, %d\n", x, y);
        fprintf(f, "i_point: %f %f %f\n", i_point.X(), i_point.Y(), i_point.Z());
        fprintf(f, "t %f\n", nearest_t);


        return DLModel(h, f, depth, max);
    }
    else {
        int r = scene.bg.X() * 255;
        int g = scene.bg.Y() * 255;
        int b = scene.bg.Z() * 255;
        Pixel bg = Pixel();
        bg.SetClamp(r, g, b);
        return bg;
    }
}*/

Pixel EvaluateRayTree(Scene scene, Ray ray, int depth, int max) {
    if (depth > max) {
        return Pixel(0, 0, 0, 1);
    }
    double nearest_t = 2000;
    Vec s_normal;
    Material s_mat;
    HitData h;
    for (int i = 0; i < scene.objects.size(); i++) {
        if (!(scene.objects[i]->CheckCollision(ray) == -999)) {
            double new_t = scene.objects[i]->CheckCollision(ray);
            if (new_t < nearest_t) {
                nearest_t = new_t;
                s_normal = scene.objects[i]->GetNormal((ray.GetOrigin() + (ray.GetDir() * nearest_t))).Normalize();
                s_mat = scene.objects[i]->GetMaterial();
            }
        }
    }
    if (nearest_t < 2000) {
        Vec i_point = (ray.GetOrigin() + (ray.GetDir() * nearest_t));
        h = HitData(s_mat, i_point, s_normal, scene, ray);
        return LModel(h, depth, max);
    }
    else {
        int r = scene.bg.X() * 255;
        int g = scene.bg.Y() * 255;
        int b = scene.bg.Z() * 255;
        Pixel bg = Pixel();
        bg.SetClamp(r, g, b);
        return bg;
    }
}

int parse(char* filename_, Scene &scene) {
    printf("parsing input file...\n");
    FILE *fp;
    long length;
    char line[1024]; //Assumes no line is longer than 1024 characters!
    int vert_c = 0;
    int norm_c = 0;

    string fileName = filename_;

    // open the file containing the scene description
    fp = fopen(fileName.c_str(), "r");

    // check for errors in opening the file
    if (fp == NULL) {
        printf("Can't open file '%s'\n", fileName.c_str());
        return 0;  //Exit
    }

    // determine the file size (this is optional -- feel free to delete the 4 lines below)
    fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
    length = ftell(fp);  // return the value of the current position
    printf("File '%s' is %ld bytes long.\n\n", fileName.c_str(), length);
    fseek(fp, 0, SEEK_SET);  // move position indicator to the start of the file

                             //Loop through reading each line
    while (fgets(line, 1024, fp)) { //Assumes no line is longer than 1024 characters!
        if (line[0] == '#') {
            printf("Skipping comment: %s", line);
            continue;
        }

        char command[100];
        int fieldsRead = sscanf(line, "%s ", command); //Read first word in the line (i.e., the command type)
        string commandStr = command;

        if (fieldsRead < 1) { //No command read
                              //Blank line
            continue;
        }
        if (commandStr == "camera") {
            double px, py, pz,
                dx, dy, dz,
                ux, uy, uz, ha;
            sscanf(line, "camera %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &px, &py, &pz, &dx, &dy, &dz, &ux, &uy, &uz, &ha);
            scene.camera = Camera(Vec(px, py, pz), Vec(ux, uy, uz), Vec(dx, dy, dz), ha);
            printf("Camera at position (%f,%f,%f), with up vector (%f,%f,%f), and direction (%f,%f,%f), with half angle %f\n\n", px, py, pz, ux, uy, uz, dx, dy, dz, ha);
        }
        else if (commandStr == "material") {
            double ar, ag, ab,
                dr, dg, db,
                sr, sg, sb, ns,
                tr, tg, tb, ior;
            sscanf(line, "material %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &ar, &ag, &ab, &dr, &dg, &db, &sr, &sg, &sb, &ns, &tr, &tg, &tb, &ior);
            scene.material = Material(Vec(dr, dg, db), Vec(ar, ag, ab), Vec(sr, sg, sb), Vec(tr, tg, tb), ns, ior);
            printf("Material with ambient color (%f,%f,%f), with diffuse color (%f,%f,%f), specular color (%f,%f,%f), and transmissive color (%f,%f,%f), with phong constant %f and index of refraction %f\n\n", ar, ag, ab, dr, dg, db, sr, sg, sb, tr, tg, tb, ns, ior);
        }
        else if (commandStr == "film_resolution") {
            int x, y;
            sscanf(line, "film_resolution %d %d", &x, &y);
            scene.width = x;
            scene.height = y;
            printf("Image resolution set to: %dX%d\n\n", x, y);
        }
        else if (commandStr == "output_image") {
            char outFile[1024];
            sscanf(line, "output_image %s", outFile);
            strcpy(scene.out_file, outFile);
            printf("Render to file named: %s\n\n", outFile);
        }
        else if (commandStr == "max_vertices") {
            int x;
            sscanf(line, "max_vertices %d", &x);
            scene.vertices = new Vec[x];
            printf("max number of vertices: (%d)\n\n", x);
        }
        else if (commandStr == "max_normals") {
            int x;
            sscanf(line, "max_normals %d", &x);
            scene.normals = new Vec[x];
            printf("max number of normals: (%d)\n\n", x);
        }
        else if (commandStr == "vertex") {
            float x, y, z;
            sscanf(line, "vertex %f %f %f", &x, &y, &z);
            scene.vertices[vert_c] = Vec(x, y, z);
            //printf("Vertex %d set to (%f,%f,%f)\n\n", vert_c, x, y, z);
            vert_c++;
        }
        else if (commandStr == "normal") {
            float x, y, z;
            sscanf(line, "normal %f %f %f", &x, &y, &z);
            scene.normals[norm_c] = Vec(x, y, z);
           // printf("normal %d set to (%f,%f,%f)\n\n", norm_c, x, y, z);
            norm_c++;
        }
        else if (commandStr == "sphere") {
            float x, y, z, r;
            sscanf(line, "sphere %f %f %f %f", &x, &y, &z, &r);
            scene.objects.push_back(new Sphere(Vec(x, y, z), r, scene.material));
            printf("Sphere at position (%f,%f,%f) with radius %f\n\n", x, y, z, r);
        }
        //if a custom camera position is going to be used it MUST be defined before any triangles
        else if (commandStr == "triangle") {
            int x, y, z;
            sscanf(line, "triangle %d %d %d", &x, &y, &z);
            scene.objects.push_back(new FTriangle(scene.vertices[x], scene.vertices[y], scene.vertices[z], scene.camera.position, scene.material));
           /* printf("triangle with vertices: (%f,%f,%f), (%f,%f,%f), (%f,%f,%f)\n\n", scene.vertices[x].X(), scene.vertices[x].Y(), scene.vertices[x].Z(),
                                                                                     scene.vertices[y].X(), scene.vertices[y].Y(), scene.vertices[y].Z(),
                                                                                     scene.vertices[z].X(), scene.vertices[z].Y(), scene.vertices[z].Z());*/
        }
        else if (commandStr == "normal_triangle") {
            int x, y, z, a, b, c;
            sscanf(line, "normal_triangle %d %d %d %d %d %d", &x, &y, &z, &a, &b, &c);
            scene.objects.push_back(new NTriangle(scene.vertices[x], scene.vertices[y], scene.vertices[z], scene.normals[a], scene.normals[b], scene.normals[c], scene.camera.position, scene.material));
           /* printf("normal triangle with vertices: (%f,%f,%f), (%f,%f,%f), (%f,%f,%f), and normals: (%f,%f,%f), (%f,%f,%f), (%f,%f,%f)\n\n", scene.vertices[x].X(), scene.vertices[x].Y(), scene.vertices[x].Z(),
                                                                                                                                             scene.vertices[y].X(), scene.vertices[y].Y(), scene.vertices[y].Z(),
                                                                                                                                             scene.vertices[z].X(), scene.vertices[z].Y(), scene.vertices[z].Z(),
                                                                                                                                             scene.normals[x].X(), scene.normals[x].Y(), scene.normals[x].Z(),
                                                                                                                                             scene.normals[y].X(), scene.normals[y].Y(), scene.normals[y].Z(),
                                                                                                                                             scene.normals[z].X(), scene.normals[z].Y(), scene.normals[z].Z());*/
        }
        else if (commandStr == "point_light") {
            float r, g, b, x, y, z;
            sscanf(line, "point_light %f %f %f %f %f %f", &r, &g, &b, &x, &y, &z);
            scene.lights.push_back(new PLight(Vec(x, y, z), Vec(r, g, b)));
            printf("Point light at position (%f,%f,%f) with color (%f,%f,%f)\n\n", x, y, z, r, g, b);
        }
        else if (commandStr == "spot_light") {
            float r, g, b, px, py, pz, dx, dy, dz, a1, a2;
            sscanf(line, "spot_light %f %f %f %f %f %f %f %f %f %f %f", &r, &g, &b, &px, &py, &pz, &dx, &dy, &dz, &a1, &a2);
            scene.lights.push_back(new SLight(Vec(px, py, pz), Vec(r, g, b), Vec(dx, dy, dz), a1, a2));
            printf("Spot light at position (%f,%f,%f) with color (%f,%f,%f), direction (%f,%f,%f), angle 1 %f, angle 2 %f\n\n", px, py, pz, r, g, b, dx, dy, dz, a1, a2);
        }
        else if (commandStr == "directional_light") {
            float r, g, b, x, y, z;
            sscanf(line, "directional_light %f %f %f %f %f %f", &r, &g, &b, &x, &y, &z);
            scene.lights.push_back(new DLight(Vec(x, y, z), Vec(r, g, b)));
            printf("Directional light with direction (%f,%f,%f) and color (%f,%f,%f)\n\n", x, y, z, r, g, b);
        }
        else if (commandStr == "ambient_light") {
            float r, g, b;
            sscanf(line, "ambient_light %f %f %f", &r, &g, &b);
            scene.ambient = Vec(r, g, b);
            printf("ambient light with color (%f,%f,%f)\n\n", r, g, b);
        }
        else if (commandStr == "background") {
            float r, g, b;
            sscanf(line, "background %f %f %f", &r, &g, &b);
            scene.bg = Vec(r, g, b);
            printf("Background color of (%f,%f,%f)\n\n", r, g, b);
        }
        else if (commandStr == "max_depth") {
            int md;

            sscanf(line, "max_depth %d", &md);
            scene.max_d = md;
            printf("max_depth: %d\n\n", md);
        }
        else {
            printf("WARNING. Unknow command: %s\n", command);
        }
    }
    printf("parsing complete\n");
    //TODO scene.printme
    printf("rendering scene\n");
    fclose(fp);
    return  1;
}


void main(int argc, char** argv) {
    typedef std::chrono::high_resolution_clock Clock;
    typedef std::chrono::milliseconds milliseconds;
    if (argc != 2) {
        cout << "Usage: ./RayTacer scenefile\n";
        exit(0);
    }
    Scene scene = Scene();
    char* fileName = argv[1];
    int p_success = parse(fileName, scene);
    if (p_success == 0) {
        exit(0);
    }
    if (p_success == 1) {
        auto t1 = std::chrono::high_resolution_clock::now();
        vector<Surface*> shapes = scene.objects;
        vector<Light*> lights = scene.lights;
        Camera camera = scene.camera;
        Vec bg = scene.bg;
        Vec ambient_l = scene.ambient;
        Vec right = (camera.up % camera.at).Normalize();
        int width = scene.width;
        int height = scene.height;
        int md = scene.max_d;
        Image* picture = new Image(width, height);

        double nearest_t = 2000;
        Vec s_normal;
        Material s_mat;
        int s_index;
        Surface *p;
        double fpz = compute_fp_z(camera.angle, height, camera.position.Z());
        HitData h;
        #pragma omp parallel for schedule(dynamic)
        for (int y = 0; y < scene.height; ++y) {
            for (int x = 0; x < scene.width; ++x) {
                double x_ = ((double)x) - (scene.width / 2);
                double y_ = -(((double)y) - (scene.height / 2));
                Vec dir = ((camera.at * fpz) + (right * x_) + (camera.up * y_)).Normalize();
                Ray ray = Ray(camera.position, dir);
                picture->SetPixel(x, y, EvaluateRayTree(scene, ray, 0, md));
            }
        }
        printf("out_file: %s\n", scene.out_file);
        picture->Write(scene.out_file);
        printf("render complete\n");
        auto t2 = std::chrono::high_resolution_clock::now();
        printf("render time (ms): %d\n", std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());
    }
}

/***************************
******Debug Main***********
***************************/
//void main(int argc, char** argv) {

    //********************************
    //Snells Law Tester
    //********************************
    /*
    Material mat1 = Material(Vec(), Vec(), Vec(), Vec(), 0, 1.5);
    
    Vec norm = Vec(0, 0, 1);
    Vec l = Vec(-.5, 0, -.5).Normalize();
    Vec l_out = Vec(-(1 / sqrt(3)), 0, -1).Normalize();
    Vec trans_in = mat1.SnellsIn(norm, l);
    Vec tir = mat1.SnellsOut(norm, l);
    Vec trans_out = mat1.SnellsOut(norm, l_out);
    printf("coords of light in material: (%f, %f, %f)\n", trans_in.X(), trans_in.Y(), trans_in.Z());
    printf("angle of refraction: %f\n\n", acos((trans_in * Vec(0, 0, -1))));

    printf("total internal reflection: (%f, %f, %f)\n\n", tir.X(), tir.Y(), tir.Z());

    printf("coords of light out of material: (%f, %f, %f)\n", trans_out.X(), trans_out.Y(), trans_out.Z());
    printf("angle of refraction (out): %f\n", acos((trans_out * Vec(0, 0, -1))));
    */
    //********************************
    //Triangle Tester
    //********************************
    /*
    FTriangle t1 = FTriangle(Vec(-0.5, 0.0, 1.0), Vec(0.5, 0.0, 1.0), Vec(0.0, 1.0, 1.0), Vec(0.0, 0.0, 0.0), Material());
    Vec p = Vec(0.0, 0.5, 1.0);
    printf("distance: %f\n", t1.GetDistance());
    double test = (p * t1.GetNormal(p)) + t1.GetDistance();
    printf("test %f\n", test);
    Ray r1 = Ray(Vec(0.0, 0.5, 0.0), Vec(0.0, 0.0, 1.0));
    Ray r2 = Ray(Vec(0.0, 1.1, 0.0), Vec(0.0, 0.0, 1.0));
    double tVal = t1.CheckCollision(r1);
    double tMiss = t1.CheckCollision(r2);
    printf("tVal: %f\n", tVal);
    printf("tMiss: %f\n", tMiss);
    */

    /*
    if (argc != 2) {
        cout << "Usage: ./RayTacer scenefile\n";
        exit(0);
    }
    Scene scene = Scene();
    char* fileName = argv[1];
    int p_success = parse(fileName, scene);
    if (p_success == 0) {
        exit(0);
    }
    if (p_success == 1) {
        FILE *f = fopen("log.txt", "w");

        vector<Surface*> shapes = scene.objects;
        vector<Light*> lights = scene.lights;
        Camera camera = scene.camera;
        Vec bg = scene.bg;
        Vec ambient_l = scene.ambient;
        Vec right = camera.up % camera.at;
        int width = scene.width;
        int height = scene.height;
        int md = scene.max_d;
        printf("height, width: %d, %d\n", width, height);
        Image* picture = new Image(width, height);

        double nearest_t = 2000;
        Vec s_normal;
        Material s_mat;
        int s_index;
        Surface *p;
        double fpz = compute_fp_z(camera.angle, height, camera.position.Z());
        fprintf(f, "fpz: %f\n", fpz);
        fprintf(f, "Camera position vector: [%f, %f, %f]\n\n", camera.position.X(), camera.position.Y(), camera.position.Z());
        HitData h;
        for (int y = 0; y < scene.height; y++) {
            for (int x = 0; x < scene.width; x++) {
                double x_ = ((double)x) - (scene.width / 2);
                double y_ = -(((double)y) - (scene.height / 2));
                Vec dir = ((camera.at * fpz) + (right * x_) + (camera.up * y_)).Normalize();
                Ray ray = Ray(camera.position, dir);
                 picture->SetPixel(x, y, EvaluateRayTree(scene, ray, f, x, y, 0, md));
            }
        }
        printf("out_file: %s\n", scene.out_file);
        picture->Write(scene.out_file);
        printf("render complete\n");
        fclose(f);
    }
}*/