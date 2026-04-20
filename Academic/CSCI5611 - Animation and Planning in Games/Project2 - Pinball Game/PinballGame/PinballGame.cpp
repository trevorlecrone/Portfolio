// PinballGame.cpp 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ALL OPENGL/SDL CODE HERE IS BASED ON PREVIOUS PROJECTS FROM WHEN I TOOK 5607 IN FALL 2017
// BOILERPLATE WAS COPIED OVER AND MODIFIED FOR RENDERING. THERE WERE STILL SEVERAL CHANGES I MADE FOR THIS
// PROJECT, INCLUDING:
// - UPDATING SHADERS TO USE ORTHOGRAPIC PROJECTION AND SUPPORT TECTURES
// - USE OF SDL_IMAGE LIBRARY TO SUPPORT PNG TEXTURES TO GREATLY SIMPLIFY RENDERING OF CIRCLES, JUST RENDER A BOX AND USE A CIRCLE TEXTURE WITH TRANSPARENCY
// - UPDATING GL DRAWING LOOP
// 
// ALL COLLISION/PHYSICS CODE IS UNIQUE, NO CODE OUTSIDE OF THIS CLASS WAS TAKEN FROM OLD PROJECTS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//Include order matters here
#include "glad/glad.h"  
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"
#include "SDL2/SDL_image.h"
//undefine main or SDL breaks
#undef main

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>

#include "Line.h"
#include "Box.h"
#include "Circle.h"
#include "Primitive.h"
#include "Pinball.h"
#include "StaticWall.h"
#include "StaticBumper.h"
#include "BlackHole.h"
#include "WhiteHole.h"
#include "Flipper.h"
#include "CollisionData.h"
#include "CollisionHandler.h"
#include "DetectionLib.h"
using namespace std;

//Globals 

const int FRAMES_PER_SECOND = 144;
const int SKIP_TICKS = 1000 / FRAMES_PER_SECOND;

const double FLIPPER_VEL = PI / (FRAMES_PER_SECOND / 3);
const double FLIPPER_L_START = -PI / 6;
const double FLIPPER_R_START = PI / 6;

float SCREEN_WIDTH = 800.0f;
float SCREEN_HEIGHT = 800.0f;
int MODE = 0;
Vec2 GRAVITY = Vec2(0.0, -3.0/ FRAMES_PER_SECOND);
double TERMINAL_V = 2800/ FRAMES_PER_SECOND;
double REACTIVE_BUMPER_ELASTICITY = 1.5;

Vec2 LAUNCHER_POWER = Vec2(0.0, 0.0);

double MAX_LAUNCHER_POWER = 20.0;

bool WAIT_FOR_START = false;

const int LINE_MAX = 1024;

// Shader sources
// Shader sources
const GLchar* vertexSource =
"#version 150 core\n"
"in vec2 position;"
"in vec2 inTexCoord;"
"out vec2 texCoord;"
"uniform mat4 model;"
"uniform mat4 proj;"
"void main() {"
"   gl_Position = model * proj * vec4(position, 0.0, 1.0);"
"   texCoord = inTexCoord;"
"}";

const GLchar* fragmentSource =
"#version 150 core\n"
"in vec2 texCoord;"
"out vec4 outColor;"
"uniform vec3 inColor;"
"uniform int useTex;"
"uniform sampler2D tex;"
"void main() {"
"    if (useTex == 1)"
"        outColor = vec4(texture(tex, texCoord));"
"    else {"
"       outColor = vec4(inColor, 1.0);"
"    }"
"}";

float g_mouse_down = false;

bool g_bTranslate = false;
bool g_bRotate = false;
bool g_bScale = false;

//load textures in
GLuint loadTexture(const char* filename) {
    SDL_Surface* surface = IMG_Load(filename);
    if (surface == NULL) { //If it failed, print the error
        printf("Error: \"%s\"\n", SDL_GetError()); return-1;
    }
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    //Load the texture into memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

    //What to do outside 0-1 range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    SDL_FreeSurface(surface);
    return tex;
}

void CopyToBuffer(Primitive* p, float* shapeData, int& offset) {
    float* pVertexArray = p->VertexArray();
    int numFloats = p->VertexArrayFloats();
    copy(pVertexArray, pVertexArray + numFloats, shapeData + offset);
    offset += numFloats;
}

void CopyFlippersToBuffer(Primitive* p, float* shapeData, int offset) {
    float* pVertexArray = p->VertexArray();
    int numFloats = p->VertexArrayFloats();
    copy(pVertexArray, pVertexArray + numFloats, shapeData + offset);
}

void Parse(char* filename_, 
    vector<Line*>& lines,
    vector<Circle*>& circles,
    vector<Box*>& boxes,
    vector<Pinball*>& pinballs,
    vector<StaticWall*>& walls,
    vector<StaticBumper*>& staticBumpers,
    vector<BlackHole*>& blackHoles, 
    vector<WhiteHole*>& whiteHoles,
    vector<Flipper*>& flippers,
    float* shapeData,
    int& bufferOffset,
    int& flipperOffset1,
    int& flipperOffset2) {

    printf("parsing input file...\n");
    FILE* fp;
    long length;
    char line[LINE_MAX];

    string fileName = filename_;

    // open the file
    fp = fopen(fileName.c_str(), "r");

    // check for errors in opening the file
    if (fp == NULL) {
        printf("Can't open file, '%s', ensure it is present and passed as the first argument after the executable name\n", fileName.c_str());
        return;  //Exit
    }
    int currentPrimitive = 0;
    //Loop through reading each line
    while (fgets(line, LINE_MAX, fp)) {
        if (line[0] == '#') {
            //printf("Skipping comment: %s\n", line);
            continue;
        }

        char command[100];
        int fieldsRead = sscanf(line, "%s ", command); //Read first word in the line (i.e., the command type)
        string typeStr = command;

        if (fieldsRead < 1) { //No command read
            //Blank line
            continue;
        }

        int id;
        if (typeStr == "MODE") {
            sscanf(line, "MODE %d", &id);
            MODE = id;
        }
        if (typeStr == "Pinball") {
            double center_x, center_y, radius, mass, elasticity, vel_x, vel_y;
            sscanf(line, "Pinball %d : %lf, %lf, %lf, %lf, %lf, %lf, %lf", &id, &center_x, &center_y, &radius, &mass, &elasticity, &vel_x, &vel_y);
            Circle* c = new Circle(Vec2(center_x, center_y), radius, id);
            circles.push_back(c);
            pinballs.push_back(new Pinball(c, mass, elasticity, Vec2(vel_x, vel_y)));
        }
        else if (typeStr == "LineWall") {
            double x_1, y_1, x_2, y_2, elasticity;
            sscanf(line, "LineWall %d : %lf, %lf, %lf, %lf, %lf", &id, &x_1, &y_1, &x_2, &y_2, &elasticity);
            Line* l = new Line(Vec2(x_1, y_1), Vec2(x_2, y_2), id);
            lines.push_back(l);
            walls.push_back(new StaticWall(l, elasticity));
            CopyToBuffer(l, shapeData, bufferOffset);
        }
        else if (typeStr == "LeftFlipper") {
            double x_1, y_1, x_2, y_2, mass, elasticity;
            sscanf(line, "LeftFlipper %d : %lf, %lf, %lf, %lf, %lf, %lf", &id, &x_1, &y_1, &x_2, &y_2, &mass, &elasticity);
            Line* l = new Line(Vec2(x_1, y_1), Vec2(x_2, y_2), id);
            lines.push_back(l);
            flippers.push_back(new Flipper(l, mass, elasticity, l->GetAnchor(), FLIPPER_L_START));
            flipperOffset1 = bufferOffset;
            CopyToBuffer(l, shapeData, bufferOffset);
        }
        else if (typeStr == "RightFlipper") {
            double x_1, y_1, x_2, y_2, mass, elasticity;
            sscanf(line, "RightFlipper %d : %lf, %lf, %lf, %lf, %lf, %lf", &id, &x_1, &y_1, &x_2, &y_2, &mass, &elasticity);
            Line* l = new Line(Vec2(x_1, y_1), Vec2(x_2, y_2), id);
            lines.push_back(l);
            flippers.push_back(new Flipper(l, mass, elasticity, l->GetAnchor(), FLIPPER_R_START));
            flipperOffset2 = bufferOffset;
            CopyToBuffer(l, shapeData, bufferOffset);
        }
        else if (typeStr == "ReactiveBumper") {
            double center_x, center_y, radius;
            sscanf(line, "ReactiveBumper %d : %lf, %lf, %lf", &id, &center_x, &center_y, &radius);
            Circle* c = new Circle(Vec2(center_x, center_y), radius, id);
            circles.push_back(c);
            staticBumpers.push_back(new StaticBumper(c, REACTIVE_BUMPER_ELASTICITY));
        }
        else if (typeStr == "Bumper") {
            double center_x, center_y, radius, elasticity;
            sscanf(line, "Bumper %d : %lf, %lf, %lf, %lf", &id, &center_x, &center_y, &radius, &elasticity);
            Circle* c = new Circle(Vec2(center_x, center_y), radius, id);
            circles.push_back(c);
            staticBumpers.push_back(new StaticBumper(c, elasticity));
        }
        else if (typeStr == "BWH") {
            int id2;
            double center_x_b, center_y_b, center_x_w, center_y_w, radius, gravity_radius;
            sscanf(line, "BWH %d %d : %lf, %lf, %lf, %lf, %lf, %lf", &id, &id2, &center_x_b, &center_y_b, &center_x_w, &center_y_w, &radius, &gravity_radius);
            Circle* cb = new Circle(Vec2(center_x_b, center_y_b), gravity_radius, id);
            Circle* cw = new Circle(Vec2(center_x_w, center_y_w), radius, id2);
            circles.push_back(cb);
            circles.push_back(cw);
            blackHoles.push_back(new BlackHole(cb, radius, Vec2(center_x_w, center_y_w)));
            whiteHoles.push_back(new WhiteHole(cw));
        }
    }
    printf("parsing complete\n");
    fclose(fp);
    return;
}

int GetLinesInScene(char* filename_) {

    printf("getting Lines from input file...\n");
    FILE* fp;
    long length;
    char line[LINE_MAX];

    string fileName = filename_;

    // open the file
    fp = fopen(fileName.c_str(), "r");
    int numLines = 0;
    // check for errors in opening the file
    if (fp == NULL) {
        printf("Can't open file, '%s', ensure it is present and passed as the first argument after the executable name\n", fileName.c_str());
        return-1;  //Exit
    }
    int currentPrimitive = 0;
    //Loop through reading each line
    while (fgets(line, LINE_MAX, fp)) {
        if (line[0] == '#') {
            //printf("Skipping comment: %s\n", line);
            continue;
        }

        char command[100];
        int fieldsRead = sscanf(line, "%s ", command); //Read first word in the line (i.e., the command type)
        string typeStr = command;

        if (fieldsRead < 1) { //No command read
            //Blank line
            continue;
        }
        
        if (typeStr == "LineWall") {
            numLines++;
        }
        else if (typeStr == "LeftFlipper") {
            numLines++;
        }
        else if (typeStr == "RightFlipper") {
            numLines++;
        }
    }
    printf("found %d lines in scene\n", numLines);
    fclose(fp);
    return numLines;
}

int main(int argc, char* argv[])
{

    // first argument is program name
    argv++, argc--;

    //Allocate memory on the graphics card to store geometry (vertex buffer object)
    vector<Circle*> circles;
    vector<Line*> lines;
    vector<Box*> boxes;

    vector<Pinball*> pinballs;
    vector<StaticWall*> staticWalls;
    vector<StaticBumper*> staticBumpers;
    vector<BlackHole*> blackHoles;
    vector<WhiteHole*> whiteHoles;
    vector<Flipper*> flippers;

    //parse(fileName, circles, lines, boxes);

    //centered, unit length objects used for rendering
    Box* unitBox = new Box(Vec2(400.0, 400.0), 1.0, 1.0, 20000);
    Circle* unitCircle = new Circle(Vec2(400.0, 400.0), 1, 30000);

    int score = 0;

    int numLines = GetLinesInScene((char*)argv[0]);

    int lineVertices = numLines * 2;
    int boxVertices = 4;
    int circleVertices = 4;

    int totalFloats = (lineVertices + boxVertices + circleVertices) * 4;

    float* shapeData = new float[totalFloats];

    int bufferOffset = 0;

    int flipperOffset1 = 0;
    int flipperOffset2 = 0;

    Parse((char*)argv[0], lines, circles, boxes, pinballs, staticWalls, staticBumpers, blackHoles, whiteHoles, flippers, shapeData, bufferOffset, flipperOffset1, flipperOffset2);

    map<int, SceneObject*> sceneObjectMap;
    map<int, Pinball*> pinballMap;
    
    for (Pinball* p : pinballs) {
        pinballMap[p->GetID()] = p;
    }

    for (StaticWall* s : staticWalls) {
        sceneObjectMap[s->GetID()] = s;
    }
    for (StaticBumper* s : staticBumpers) {
        sceneObjectMap[s->GetID()] = s;
    }
    for (BlackHole* bh : blackHoles) {
        sceneObjectMap[bh->GetID()] = bh;
    }
    for (WhiteHole* wh : whiteHoles) {
        sceneObjectMap[wh->GetID()] = wh;
    }
    for (Flipper* f : flippers) {
        sceneObjectMap[f->GetID()] = f;
    }

   
    //int flipperOffset1 = bufferOffset;
    //CopyToBuffer(l6, shapeData, bufferOffset);
    //int flipperOffset2 = bufferOffset;
    //CopyToBuffer(l7, shapeData, bufferOffset);
    CopyToBuffer(unitBox, shapeData, bufferOffset);
    CopyToBuffer(unitCircle, shapeData, bufferOffset);

    SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)

    //Ask SDL to get a recent version of OpenGL (3.2 or greater)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

    //Create a window (offsetx, offsety, width, height, flags)
    SDL_Window* window = SDL_CreateWindow("Pinball game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(window);

    //OpenGL functions using glad library
    if (gladLoadGLLoader(SDL_GL_GetProcAddress)) {
        printf("OpenGL loaded\n");
        printf("Vendor:   %s\n", glGetString(GL_VENDOR));
        printf("Renderer: %s\n", glGetString(GL_RENDERER));
        printf("Version:  %s\n", glGetString(GL_VERSION));
    }
    else {
        printf("ERROR: Failed to initialize OpenGL context.\n");
        return -1;
    }

    //Build a Vertex Array Object. This stores the VBO and attribute mappings in one object
    GLuint vao;
    //Create a VAO
    glGenVertexArrays(1, &vao);
    //Bind the above created VAO to the current context 
    glBindVertexArray(vao);

    GLuint vbo[1];
    //Create 1 buffer called vbo
    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

    //compile shaders, confirm success
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    GLint status;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char buffer[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
        printf("Vertex Shader Compile Failed. Info:\n\n%s\n", buffer);
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char buffer[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
        printf("Fragment Shader Compile Failed. Info:\n\n%s\n", buffer);
    }

    //Join the vertex and fragment shaders together into one program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    // set output
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    //run the linker
    glLinkProgram(shaderProgram);
    //Set the active shader (only one can be used at a time)
    glUseProgram(shaderProgram);

    //Projection matrix, makes translations and scaling simpler
    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
    glm::mat4 proj = glm::ortho(0.0f, SCREEN_WIDTH, 0.0f, SCREEN_HEIGHT, -1.0f, 1.0f);
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

    GLint uniModel = glGetUniformLocation(shaderProgram, "model");
    glm::mat4 model;
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

    GLint uniUseTex = glGetUniformLocation(shaderProgram, "useTex");

    //initialize the textures
    GLuint textures[7];
    textures[0] = loadTexture("textures/pinballNoGap.png");
    textures[1] = loadTexture("textures/reactiveBumper.png");
    textures[2] = loadTexture("textures/reactiveBumperActive.png");
    textures[3] = loadTexture("textures/Bumper.png");
    textures[4] = loadTexture("textures/BlackHole.png");
    textures[5] = loadTexture("textures/WhiteHole.png");
    textures[6] = loadTexture("textures/Plunger.png");

    //Tell OpenGL how to set fragment shader input for position and texture coordinates
    //Attribute, vals/attrib., type, normalized?, stride(size of entire vertex entry), offset
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(posAttrib);
    GLint texAttrib = glGetAttribLocation(shaderProgram, "inTexCoord");
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(texAttrib);

    //enable alpha blend and multiSampling
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    GLint uniColor = glGetUniformLocation(shaderProgram, "inColor");


    //Event Loop (Loop forever processing each event as fast as possible)
    SDL_Event windowEvent;
    bool done = false;
    bool falling = true;
    DWORD next_game_tick = GetTickCount64();
    int sleep_time = 0;
    if (MODE == 2) GRAVITY = Vec2(0.0,0.0);
    while (!done) {
        //Process input events (e.g., mouse & keyboard)
        while (SDL_PollEvent(&windowEvent)) {  
            if (windowEvent.type == SDL_QUIT) done = true;
            //List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
                done = true; //Exit event loop
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_p) {
                textures[0] = loadTexture("textures/pirate.png");
                textures[1] = loadTexture("textures/pirate.png");
                textures[2] = loadTexture("textures/pirate.png");
                textures[3] = loadTexture("textures/pirate.png");
                textures[4] = loadTexture("textures/pirate.png");
                textures[5] = loadTexture("textures/pirate.png");
                textures[6] = loadTexture("textures/pirate.png");
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_g) {
                GRAVITY = Vec2(0.0, -3.0 / FRAMES_PER_SECOND);
            }
            if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_LCTRL) {
                if (flippers[0]->angle != FLIPPER_R_START) {
                    flippers[0]->rotationalVelocity = FLIPPER_VEL;
                }
            }
            if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_RCTRL) {
                if (flippers[1]->angle != FLIPPER_L_START) {
                    flippers[1]->rotationalVelocity = -FLIPPER_VEL;
                }
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_LCTRL) {
                flippers[0]->rotationalVelocity = -FLIPPER_VEL;
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_RCTRL) {
                flippers[1]->rotationalVelocity = FLIPPER_VEL;
            }
            if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_DOWN) {
                if (LAUNCHER_POWER.y < MAX_LAUNCHER_POWER) {
                    LAUNCHER_POWER.y += 0.5;
                }
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_DOWN) {
                for (Pinball* pb : pinballs) {
                    if (pb->p->GetAnchor().x < 686.0 && pb->p->GetAnchor().x > 664.0 && pb->p->GetAnchor().y < 216.0 && pb->p->GetAnchor().y > 214.0) {
                        pb->SetVelocity(LAUNCHER_POWER);
                    }
                }
                LAUNCHER_POWER.y = 0.0;
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_r) {

                lines.clear();
                circles.clear();
                boxes.clear();
                pinballs.clear();
                staticWalls.clear();
                staticBumpers.clear();
                blackHoles.clear();
                whiteHoles.clear();
                sceneObjectMap.clear();
                pinballMap.clear();
                flippers.clear();

                bufferOffset = 0;
                flipperOffset1 = 0;
                flipperOffset2 = 0;

                Parse((char*)argv[0], lines, circles, boxes, pinballs, staticWalls, staticBumpers, blackHoles, whiteHoles, flippers, shapeData, bufferOffset, flipperOffset1, flipperOffset2);

                for (Pinball* p : pinballs) {
                    pinballMap[p->GetID()] = p;
                }

                for (StaticWall* s : staticWalls) {
                    sceneObjectMap[s->GetID()] = s;
                }
                for (StaticBumper* s : staticBumpers) {
                    sceneObjectMap[s->GetID()] = s;
                }
                for (BlackHole* bh : blackHoles) {
                    sceneObjectMap[bh->GetID()] = bh;
                }
                for (WhiteHole* wh : whiteHoles) {
                    sceneObjectMap[wh->GetID()] = wh;
                }
                for (Flipper* f : flippers) {
                    sceneObjectMap[f->GetID()] = f;
                }
                WAIT_FOR_START = false;
                score = 0;
            }
        }

        flippers[0]->Rotate();
        flippers[1]->Rotate();
        CopyFlippersToBuffer(flippers[0]->p, shapeData, flipperOffset1);
        CopyFlippersToBuffer(flippers[1]->p, shapeData, flipperOffset2);
        vector<CollisionData> collidingPrimitives;
        DetectionLib::DetectCollisions(circles, lines, boxes, collidingPrimitives);
        for (Pinball* pb : pinballs) {
            Vec2 vel = pb->Velocity();
            Vec2 nextVel = vel + GRAVITY;
            if (nextVel * nextVel <= TERMINAL_V || nextVel * nextVel < vel * vel) {
                pb->SetVelocity(nextVel);
            }
            
            int pinballId = pb->GetID();
            for (CollisionData c : collidingPrimitives) {
                if (c.HasPrimitive(pinballId)) {
                    int otherID = c.GetOtherPrimitive(pb->GetID());
                    if (pinballMap.count(otherID)) {
                        CollisionHandler::ResolveCollision(pb, pinballMap[otherID], MODE, score);
                    }
                    else if (sceneObjectMap.count(otherID)) {
                        CollisionHandler::ResolveCollision(pb, sceneObjectMap[otherID], MODE, score);
                    }
                    //gross lambda to work around linking issues, don't want to double-evaluate pinball-pinball collisions
                    auto handled = std::find_if(collidingPrimitives.begin(), collidingPrimitives.end(),
                        [pinballId, otherID](CollisionData cd)
                        {
                            return (cd.p1->GetID() == pinballId || cd.p2->GetID() == pinballId) &&
                        (cd.p1->GetID() == otherID || cd.p2->GetID() == otherID); 
                        });
                    if (handled != collidingPrimitives.end()) collidingPrimitives.erase(handled);
                }
            }
            vel = pb->Velocity();
            pb->p->SetAnchor(pb->p->GetAnchor() + vel);
            if (vel * vel > TERMINAL_V) {
                pb->SetVelocity(vel * 0.9);
            }
        }
        for (StaticBumper* sb : staticBumpers) {
            if (sb->isActive > 0) {
                sb->isActive--;
            }
        }
        for (Pinball* pb : pinballs) {
            if (pb->p->GetAnchor().y <= -15.0) {
                auto deadBall = std::find_if(pinballs.begin(), pinballs.end(),
                    [pb](Pinball* pb2)
                    {
                        return pb->GetID() == pb2->GetID();
                            
                    });
                if (deadBall != pinballs.end()) pinballs.erase(deadBall);
            }
        }
        if (pinballs.empty() && !WAIT_FOR_START) {
            printf("GAME OVER! Your score was: %d. Press 'r' to try again\n", score);
            WAIT_FOR_START = true;
        }
       
        //upload verticies
        glBufferData(GL_ARRAY_BUFFER, totalFloats * sizeof(float), shapeData, GL_DYNAMIC_DRAW);

        //set background color
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT);

        //set uniform color
        glUniform3f(uniColor, 0.5f, 0.5f, 1.0f);

        glLineWidth((GLfloat)2.0);
        glUniform1i(uniUseTex, 0);
        model = glm::mat4();
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_LINES, 0, lineVertices);
        float trans_x, trans_y;
        if (MODE == 1) {
            model = glm::mat4();
            glUniform1i(uniUseTex, 1);
            trans_x = (675.0 - 400.0) / 400.0;
            trans_y = (150.0 - 400.0) / 400.0;
            model = glm::translate(model, glm::vec3(trans_x, trans_y, 1.0f));
            model = glm::scale(model, glm::vec3(50.0, 100.0 / (1.0 + (LAUNCHER_POWER.y/2)), 1.0f));
            glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
            glBindTexture(GL_TEXTURE_2D, textures[6]);
            glDrawArrays(GL_TRIANGLE_STRIP, lineVertices, boxVertices);
        }

        for (StaticBumper* sb : staticBumpers) {
            Circle* c = (Circle*)sb->p;
            glUniform1i(uniUseTex, 1);
            model = glm::mat4();
            trans_x = (c->GetAnchor().x - 400.0) / 400.0;
            trans_y = (c->GetAnchor().y - 400.0) / 400.0;
            model = glm::translate(model, glm::vec3(trans_x, trans_y, 1.0f));
            model = glm::scale(model, glm::vec3(c->GetRadius(), c->GetRadius(), 1.0f));
            glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
            if (sb->elasticity == REACTIVE_BUMPER_ELASTICITY) {
                if (sb->isActive > 0)
                {
                    glBindTexture(GL_TEXTURE_2D, textures[2]);
                }
                else {
                    glBindTexture(GL_TEXTURE_2D, textures[1]);
                }
            }
            else {
                glBindTexture(GL_TEXTURE_2D, textures[3]);
            }
            glDrawArrays(GL_TRIANGLE_STRIP, lineVertices + boxVertices, circleVertices);
        }

        for (BlackHole* bh : blackHoles) {
            Circle* c = (Circle*)bh->p;
            glUniform1i(uniUseTex, 1);
            model = glm::mat4();
            trans_x = (c->GetAnchor().x - 400.0) / 400.0;
            trans_y = (c->GetAnchor().y - 400.0) / 400.0;
            model = glm::translate(model, glm::vec3(trans_x, trans_y, 1.0f));
            model = glm::scale(model, glm::vec3(bh->renderRadius, bh->renderRadius, 1.0f));
            glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
            glBindTexture(GL_TEXTURE_2D, textures[4]);
            glDrawArrays(GL_TRIANGLE_STRIP, lineVertices + boxVertices, circleVertices);
        }
        for (WhiteHole* wh : whiteHoles) {
            Circle* c = (Circle*)wh->p;
            glUniform1i(uniUseTex, 1);
            model = glm::mat4();
            trans_x = (c->GetAnchor().x - 400.0) / 400.0;
            trans_y = (c->GetAnchor().y - 400.0) / 400.0;
            model = glm::translate(model, glm::vec3(trans_x, trans_y, 1.0f));
            model = glm::scale(model, glm::vec3(c->GetRadius(), c->GetRadius(), 1.0f));
            glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
            glBindTexture(GL_TEXTURE_2D, textures[5]);
            glDrawArrays(GL_TRIANGLE_STRIP, lineVertices + boxVertices, circleVertices);
        }

        for (Pinball* pb : pinballs) {
            Circle* c = (Circle*)pb->p;
            glUniform1i(uniUseTex, 1);
            model = glm::mat4();
            trans_x = (c->GetAnchor().x - 400.0) / 400.0;
            trans_y = (c->GetAnchor().y - 400.0) / 400.0;
            model = glm::translate(model, glm::vec3(trans_x, trans_y, 1.0f));
            model = glm::scale(model, glm::vec3(c->GetRadius(), c->GetRadius(), 1.0f));
            glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
            glBindTexture(GL_TEXTURE_2D, textures[0]);
            glDrawArrays(GL_TRIANGLE_STRIP, lineVertices + boxVertices, circleVertices);
        }
        next_game_tick += SKIP_TICKS;
        sleep_time = next_game_tick - GetTickCount64();
        if (sleep_time >= 0) {
            Sleep(sleep_time/2);
        }
        SDL_GL_SwapWindow(window); //Double buffering   
    }

    glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    glDeleteBuffers(1, vbo);
    glDeleteVertexArrays(1, &vao);


    //Clean Up
    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}
