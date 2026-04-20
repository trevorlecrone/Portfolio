#include "GL/glew.h"   //Include order can matter here
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"

#define GLEW_STATIC
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "Vec.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "LevelParser.h"
using namespace std;

bool saveOutput = false;
float timePast = 0;

//load the level parser, and initialize some important data
LevelParser l = LevelParser();
string levelArray[6] = { "resources/levels/level1.txt", "resources/levels/level2.txt", "resources/levels/level3.txt", "resources/levels/level4.txt", "resources/levels/level5.txt", "resources/levels/youwin.txt" };
bool heldKeys[5] = { false, false, false, false, false };
glm::vec3 colors[5] = { glm::vec3(1.0f, 0.5f, 1.0f), glm::vec3(0.0f, 0.4f, 1.0f), glm::vec3(1.0f, 0.3f, 0.0f), glm::vec3(0.7f, 1.0f, 1.0f), glm::vec3(0.3f, 0.3f, 0.3f) };
float door_z[5] = { 0.125, 0.125, 0.125, 0.125 };
string keys = "abcde";
string doors = "ABCDE";

//camera and direction coordinates
Vec cam = Vec();
Vec atP = Vec(0.0,0.0,0.0);
Vec atV = (atP - cam).Normalize();
Vec up = Vec(0.0, 0.0, 1.0);
Vec rght = atV % up;
//last safe camerea position, and the collision camera
Vec sCam = Vec();
Vec cCam = Vec();
//floor transformations
float f_x_scale;
float f_y_scale;


//load textures in
GLuint loadTexture(char* filename) {
    SDL_Surface* surface = SDL_LoadBMP(filename);
    if (surface == NULL) { //If it failed, print the error
        printf("Error: \"%s\"\n", SDL_GetError()); return-1;
    }
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    //Load the texture into memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels);

    //What to do outside 0-1 range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    SDL_FreeSurface(surface);
    return tex;
}

int collision_check(int x_c, int y_c, int width, int height, string* &matrix) {
    if (x_c >= height || y_c >= width || x_c < 0 || y_c < 0) {
        x_c = height / 2;
        y_c = width / 2;
        return 0;
    }
    else if (x_c >= 0 && x_c < height  && y_c >= 0 && y_c < width) {
        int key = (int)keys.find(matrix[x_c][y_c]);
        int door = (int)doors.find(matrix[x_c][y_c]);
        if (matrix[x_c][y_c] == 'W') {
            return 0;
        }
        else if (matrix[x_c][y_c] == 'G') {
            return 2;
        }
        else if (key >= 0) {
            matrix[x_c][y_c] = '0';
            heldKeys[key] = true;
            return 1;
        }
        else if (door >= 0) {
            if (heldKeys[door]) {
                return 1;
            }
            else {
                return 0;
            }
        }
        else {
            return 1;
        }
    }
    else {
        return 1;
    }
}

string* initialize_level(const char* filename, int &width, int &height, float &h_width, float &h_height) {
    string* lMatrix;
    lMatrix = l.parse(filename, width, height);
    for (int k = 0; k < 5; k++) {
        heldKeys[k] = false;
        door_z[k] = 0.125;
    }
    //coords useful for transforms
    h_width = floor(((float)width) / 2.0f);
    h_height = floor(((float)height) / 2.0f);

    printf("Width: %d  Height: %d \n", width, height);
    for (int i = 0; i < height; i++) {
        //need to use cout since this is a c++ style string
        cout << lMatrix[i] << endl;
    }
    float player_x{}, player_y{};
    for (int i = 0; i < height; i++) {
        int s_coord = lMatrix[i].find_first_of('S');
        if (s_coord != -1) {
            player_x = -2 * (i - (h_height- 0.25));
            player_y = -2 * (s_coord - (h_width - 0.25));
            i = height;
        }
    }
    if (width % 2 == 0) {
        h_width = h_width - 0.5f;
    }
    if (height % 2 == 0) {
        h_height = h_height - 0.5f;
    }
    //initialize camera and other vectors
    cam = Vec(player_x, player_y, 0.0f);
    atV = (atP - cam).Normalize();
    up = Vec(0.0, 0.0, 1.0);
    rght = atV % up;
    //last safe camerea position, and the collision camera
    sCam = Vec();
    cCam = Vec();
    //floor transformations
    f_x_scale = (height * 2.0f) + 4.0f;
    f_y_scale = (width * 2.0f) + 4.0f;
    return lMatrix;
}

bool DEBUG_ON = true;
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName);
bool fullscreen = false;
void Win2PPM(int width, int height);

int main(int argc, char *argv[]) {

    SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)
    SDL_ShowCursor(SDL_DISABLE);
    //Ask SDL to get a recent version of OpenGL (3.2 or greater)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    //parse the level file
    int width, height;
    int level_index = 0;
    float half_w, half_h;
    string * levelMatrix;
    string levelString = levelArray[level_index];
    const char* levelFile = levelString.c_str();
    levelMatrix = initialize_level(levelFile, width, height, half_w, half_h);
    

    //Create a window (offsetx, offsety, width, height, flags)
    SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, 1200, 900, SDL_WINDOW_OPENGL);
    //SDL_SetRelativeMouseMode(SDL_TRUE);
    //The above window cannot be resized which makes some code slightly easier.
    //Below show how to make a full screen window or allow resizing
    //SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 0, 0, 800, 600, SDL_WINDOW_FULLSCREEN|SDL_WINDOW_OPENGL);
    //SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, 800, 600, SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);
    //SDL_Window* window = SDL_CreateWindow("My OpenGL Program",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,0,0,SDL_WINDOW_FULLSCREEN_DESKTOP|SDL_WINDOW_OPENGL); //Boarderless window "fake" full screen


    //Create a context to draw in
    SDL_GLContext context = SDL_GL_CreateContext(window);


    //GLEW loads new OpenGL functions
    glewExperimental = GL_TRUE; //Use the new way of testing which methods are supported
    glewInit();

    //Build a Vertex Array Object. This stores the VBO and attribute mappings in one object
    GLuint vao;
    glGenVertexArrays(1, &vao); //Create a VAO
    glBindVertexArray(vao); //Bind the above created VAO to the current context


                            //Load Model 1
    ifstream modelFile;
    modelFile.open("resources/models/cube.txt");
    int numLines = 0;
    modelFile >> numLines;
    float* model1 = new float[numLines];
    for (int i = 0; i < numLines; i++) {
        modelFile >> model1[i];
    }
    printf("%d\n", numLines);
    int numTris1 = numLines / 8;
    modelFile.close();

    //Load Model 2
    modelFile.open("resources/models/key.txt");
    numLines = 0;
    modelFile >> numLines;
    float* model2 = new float[numLines];
    for (int i = 0; i < numLines; i++) {
        modelFile >> model2[i];
    }
    printf("%d\n", numLines);
    int numTris2 = numLines / 8;
    modelFile.close();

    //Load Model 3
    modelFile.open("resources/models/goal.txt");
    numLines = 0;
    modelFile >> numLines;
    float* model3 = new float[numLines];
    for (int i = 0; i < numLines; i++) {
        modelFile >> model3[i];
    }
    printf("%d\n", numLines);
    int numTris3 = numLines / 8;
    modelFile.close();


    //load models in the same way that it was done in the provided source
    float* modelData = new float[(numTris1 + numTris2 + numTris3) * 8];
    copy(model1, model1 + numTris1 * 8, modelData);
    copy(model2, model2 + numTris2 * 8, modelData + numTris1 * 8);
    copy(model3, model3 + numTris3 * 8, modelData + (numTris1 + numTris2) * 8);
    int totalNumTris = numTris1 + numTris2 + numTris3;

    //initialize the textures
    GLuint textures[3];
    textures[0] = loadTexture("resources/textures/brick.bmp");
    textures[1] = loadTexture("resources/textures/gray_concrete.bmp");
    textures[2] = loadTexture("resources/textures/pirate.bmp");

    //Allocate memory on the graphics card to store geometry (vertex buffer object)
    GLuint vbo[1];
    glGenBuffers(1, vbo);  //Create 1 buffer called vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
                                           //GL_STATIC_DRAW means we won't change the geometry, GL_DYNAMIC_DRAW = geometry changes infrequently
                                           //GL_STREAM_DRAW = geom. changes frequently.  This effects which types of GPU memory is used

    int shaderProgram = InitShader("vertexTex.glsl", "fragmentTex.glsl");
    glUseProgram(shaderProgram); //Set the active shader (only one can be used at a time)



    glEnable(GL_DEPTH_TEST);

    //Event Loop (Loop forever processing each event as fast as possible)
    SDL_Event windowEvent;
    while (true) {
        glm::vec3 glmAt = glm::vec3(atV.X(), atV.Y(), atV.Z());
        if (SDL_PollEvent(&windowEvent)) {
            if (windowEvent.type == SDL_QUIT) {
                SDL_ShowCursor(SDL_ENABLE);
                break;
            }
            //List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
            //Scancode referes to a keyboard position, keycode referes to the letter (e.g., EU keyboards)
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
                break; //Exit event loop
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f) { //If "f" is pressed
                fullscreen = !fullscreen;
                SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0); //Toggle fullscreen 
            }
            //some debug statement keys
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_x) { 
                cam.PrintMe();
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_1) {
                level_index = 0;
                levelString = levelArray[level_index];
                levelFile = levelString.c_str();
                levelMatrix = initialize_level(levelFile, width, height, half_w, half_h);
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_2) {
                level_index = 1;
                levelString = levelArray[level_index];
                levelFile = levelString.c_str();
                levelMatrix = initialize_level(levelFile, width, height, half_w, half_h);
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_3) {
                level_index = 2;
                levelString = levelArray[level_index];
                levelFile = levelString.c_str();
                levelMatrix = initialize_level(levelFile, width, height, half_w, half_h);
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_4) {
                level_index = 3;
                levelString = levelArray[level_index];
                levelFile = levelString.c_str();
                levelMatrix = initialize_level(levelFile, width, height, half_w, half_h);
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_5) {
                level_index = 4;
                levelString = levelArray[level_index];
                levelFile = levelString.c_str();
                levelMatrix = initialize_level(levelFile, width, height, half_w, half_h);
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_p) {
                textures[0] = loadTexture("resources/textures/pirate.bmp");
                textures[1] = loadTexture("resources/textures/pirate.bmp");
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_o) {
                textures[0] = loadTexture("resources/textures/wut.bmp");
                textures[1] = loadTexture("resources/textures/wut.bmp");
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_i) {
                textures[0] = loadTexture("resources/textures/ea.bmp");
                textures[1] = loadTexture("resources/textures/ea.bmp");
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_n) {
                textures[0] = loadTexture("resources/textures/brick.bmp");
                textures[1] = loadTexture("resources/textures/gray_concrete.bmp");
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_z) {
                cam.PrintMe();
                int m_x = floor((height - cam.X()) / 2);
                int m_y = floor((width - cam.Y()) / 2);
                cout << heldKeys[0] << endl;
                printf("m_x: %d  m_y: %d\n", m_x, m_y);
                printf("%c\n", levelMatrix[m_x][m_y]);
                printf("Width: %d  Height: %d \n", width, height);
                for (int i = 0; i < height; i++) {
                    //need to use cout since this is a c++ style string
                    cout << levelMatrix[i] << endl;
                }
            }
        }

        const Uint8* pressedKeys = SDL_GetKeyboardState(NULL);
        ////////////
        if (pressedKeys[SDL_SCANCODE_W]) { //If "w" is down
            sCam = cam;
            cCam = cam + atV *.08;
            cam = cam + atV *.04;
            int m_x = floor((height - cCam.X()) / 2);
            int m_y = floor((width - cCam.Y()) / 2);
            int cCheck = collision_check(m_x, m_y, width, height, levelMatrix);
            if (cCheck == 0) {
                cam = sCam;
            }
            if (cCheck == 2) {
                level_index++;
                if (level_index == 5) {
                    textures[0] = loadTexture("resources/textures/youwin.bmp");
                }
                levelString = levelArray[level_index];
                levelFile = levelString.c_str();
                levelMatrix = initialize_level(levelFile, width, height, half_w, half_h);
            }
        }
        if (pressedKeys[SDL_SCANCODE_S]) { //If "s" is down
            sCam = cam;
            cCam = cam - atV *.08;
            cam = cam - atV *.04;
            int m_x = floor((height - cCam.X()) / 2);
            int m_y = floor((width - cCam.Y()) / 2);
            int cCheck = collision_check(m_x, m_y, width, height, levelMatrix);
            if (cCheck == 0) {
                cam = sCam;
            }
            if (cCheck == 2) {
                level_index++;
                if (level_index == 5) {
                    textures[0] = loadTexture("resources/textures/youwin.bmp");
                }
                levelString = levelArray[level_index];
                levelFile = levelString.c_str();
                levelMatrix = initialize_level(levelFile, width, height, half_w, half_h);
            }
        }
        if (pressedKeys[SDL_SCANCODE_A]) { //If "a" is down
            sCam = cam;
            cCam = cam - rght *.08;
            cam = cam - rght *.04;
            int m_x = floor((height - cCam.X()) / 2);
            int m_y = floor((width - cCam.Y()) / 2);
            int cCheck = collision_check(m_x, m_y, width, height, levelMatrix);
            if (cCheck == 0) {
                cam = sCam;
            }
            if (cCheck == 2) {
                level_index++;
                if (level_index == 5) {
                    textures[0] = loadTexture("resources/textures/youwin.bmp");
                }
                levelString = levelArray[level_index];
                levelFile = levelString.c_str();
                levelMatrix = initialize_level(levelFile, width, height, half_w, half_h);
            }
        }
        if (pressedKeys[SDL_SCANCODE_D]) { //If "d" is down
            sCam = cam;
            cCam = cam + rght *.08;
            cam = cam + rght *.04;
            int m_x = floor((height - cCam.X()) / 2);
            int m_y = floor((width - cCam.Y()) / 2);
            int cCheck = collision_check(m_x, m_y, width, height, levelMatrix);
            if (cCheck == 0) {
                cam = sCam;
            }
            if (cCheck == 2) {
                level_index++;
                if (level_index == levelArray->size() - 1) {
                    textures[0] = loadTexture("resources/textures/youwin.bmp");
                }
                levelString = levelArray[level_index];
                levelFile = levelString.c_str();
                levelMatrix = initialize_level(levelFile, width, height, half_w, half_h);
            }
        }
        if (pressedKeys[SDL_SCANCODE_LEFT]) { //If "d" is down
            glmAt = glm::rotateZ(glmAt, 0.033f);
            atV = Vec(glmAt.x, glmAt.y, glmAt.z);
            rght = atV % up;
        }
        if (pressedKeys[SDL_SCANCODE_RIGHT]) { //If "d" is down
            glmAt = glm::rotateZ(glmAt, -0.033f);
            atV = Vec(glmAt.x, glmAt.y, glmAt.z);
            rght = atV % up;
        }
        if (!saveOutput) timePast = SDL_GetTicks() / 1000.f;
        if (saveOutput) timePast += .07; //Fix framerate at 14 FPS

        glUseProgram(shaderProgram);

        glBufferData(GL_ARRAY_BUFFER, totalNumTris * 8 * sizeof(float), modelData, GL_STATIC_DRAW);

        //Tell OpenGL how to set fragment shader input 
        GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
        //Attribute, vals/attrib., type, normalized?, stride, offset
        //Binds to VBO current GL_ARRAY_BUFFER 
        glEnableVertexAttribArray(posAttrib);

        //GLint colAttrib = glGetAttribLocation(shaderProgram, "inColor");
        //glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
        //glEnableVertexAttribArray(colAttrib);

        GLint normAttrib = glGetAttribLocation(shaderProgram, "inNormal");
        glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(normAttrib);

        GLint texAttrib = glGetAttribLocation(shaderProgram, "inTexcoord");
        glEnableVertexAttribArray(texAttrib);
        glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE,
            8 * sizeof(float), (void*)(3 * sizeof(float)));

        GLint uniModel = glGetUniformLocation(shaderProgram, "model");
        GLint uniView = glGetUniformLocation(shaderProgram, "view");
        GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
        GLint uniUseTex = glGetUniformLocation(shaderProgram, "useTex");
        GLint uniInColor = glGetUniformLocation(shaderProgram, "inColor");

        // Clear the screen to default color
        glClearColor(.2f, 0.4f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = glm::lookAt(
            glm::vec3(cam.X(), cam.Y(), cam.Z()),  //Cam Position
            glm::vec3(cam.X() + atV.X(), cam.Y() + atV.Y(), cam.Z() + atV.Z()),  //Look at point
            glm::vec3(0.0f, 0.0f, 1.0f)); //Up

        glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 proj = glm::perspective(3.14f / 4, 800.0f / 600.0f, 0.025f, 20.0f); //FOV, aspect, near, far
        glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

        //Draw the ground
        //glUniform1d(uniUseTex, 1);
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        glm::mat4 model;
        model = glm::scale(model, glm::vec3(f_x_scale, f_y_scale, 0.5f));
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.0f));
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(uniUseTex, 1);
        glDrawArrays(GL_TRIANGLES, 0, numTris1);

        //loop to draw the walls

        for (int i = -1; i <= height; i++) {
            for (int j = -1; j <= width; j++) {
                if (i < 0 || j < 0 || i == height || j == width || levelMatrix[i][j] == 'W') {
                    float trans_x = -2 * (i - half_h);
                    float trans_y = -2 * (j - half_w);
                    model = glm::mat4();
                    model = glm::translate(model, glm::vec3(trans_x, trans_y, 0.125f));
                    model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f)); //An example of scale
                    glBindTexture(GL_TEXTURE_2D, textures[0]);
                    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
                    glUniform1i(uniUseTex, 1);
                    glDrawArrays(GL_TRIANGLES, 0, numTris1);
                }
                else {
                    int key = keys.find(levelMatrix[i][j]);
                    int door = doors.find(levelMatrix[i][j]);
                    if (key >= 0) {
                        float trans_x = -2 * (i - half_h);
                        float trans_y = -2 * (j - half_w);
                        model = glm::mat4();
                        model = glm::translate(model, glm::vec3(trans_x, trans_y, (0.1f *sinf(1.5f * timePast)) - 0.05f));
                        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
                        model = glm::rotate(model, 3.14159f / 2, glm::vec3(1.0f, 0.0f, 0.0f));
                        model = glm::rotate(model, timePast * .6f * 3.14f / 4, glm::vec3(0.0f, 1.0f, 0.0f));
                        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform3fv(uniInColor, 1, glm::value_ptr(colors[key]));
                        glUniform1i(uniUseTex, 0);
                        glDrawArrays(GL_TRIANGLES, numTris1, numTris2);
                    }
                    if (door >= 0) {
                        float trans_x = -2 * (i - half_h);
                        float trans_y = -2 * (j - half_w);
                        float trans_z = 0.125;
                        if (heldKeys[door]) {
                            if (door_z[door] <= 2.125) {
                                door_z[door] = door_z[door] + 0.01;
                            }
                            trans_z = door_z[door];
                        }
                        model = glm::mat4();
                        model = glm::translate(model, glm::vec3(trans_x, trans_y, trans_z));
                        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
                        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform3fv(uniInColor, 1, glm::value_ptr(colors[door]));
                        glUniform1i(uniUseTex, 0);
                        glDrawArrays(GL_TRIANGLES, 0, numTris1);
                    }
                    if (levelMatrix[i][j] == 'G') {
                        float trans_x = -2 * (i - half_h);
                        float trans_y = -2 * (j - half_w);
                        model = glm::mat4();
                        model = glm::translate(model, glm::vec3(trans_x, trans_y, (0.1f *sinf(1.5f * timePast)) - 0.05f));
                        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
                        model = glm::rotate(model, 3.14159f / 2, glm::vec3(1.0f, 0.0f, 0.0f));
                        model = glm::rotate(model, timePast * .6f * 3.14f / 4, glm::vec3(0.0f, 1.0f, 0.0f));
                        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform3fv(uniInColor, 1, glm::value_ptr(glm::vec3(0.5 + (sin(timePast * 0.1)/2), 0.5 + (sin(timePast * 0.3) / 2), 0.5 + (sin(timePast * 0.15) / 2))));
                        glUniform1i(uniUseTex, 0);
                        glDrawArrays(GL_TRIANGLES, numTris1 + numTris2, numTris3);
                    }
                    for (int i = 0; i < 5; i++) {
                        if (heldKeys[i]) {
                            model = glm::mat4();
                            float r_factor = ((-2 + i) * 0.005);
                            float key_t_x = cam.X() + (0.03 * atV.X()) + (r_factor * rght.X());
                            float key_t_y = cam.Y() + (0.03 * atV.Y()) + (r_factor * rght.Y());
                            model = glm::translate(model, glm::vec3(key_t_x, key_t_y, cam.Z() - 0.01));
                            model = glm::scale(model, glm::vec3(0.001f, 0.001f, 0.001f));
                            model = glm::rotate(model, 3.14159f / 2, glm::vec3(1.0f, 0.0f, 0.0f));
                            model = glm::rotate(model, timePast * .6f * 3.14f / 4, glm::vec3(0.0f, 1.0f, 0.0f));
                            glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
                            glUniform3fv(uniInColor, 1, glm::value_ptr(colors[i]));
                            glUniform1i(uniUseTex, 0);
                            glDrawArrays(GL_TRIANGLES, numTris1, numTris2);
                        }
                    }
                }
            }
        }

        if (saveOutput) Win2PPM(800, 600);


        SDL_GL_SwapWindow(window); //Double buffering
    }

    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, vbo);
    glDeleteVertexArrays(1, &vao);

    //Clean Up
    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}

// Create a NULL-terminated string by reading the provided file
static char* readShaderSource(const char* shaderFile)
{
    FILE *fp;
    long length;
    char *buffer;

    // open the file containing the text of the shader code
    fp = fopen(shaderFile, "r");

    // check for errors in opening the file
    if (fp == NULL) {
        printf("can't open shader source file %s\n", shaderFile);
        return NULL;
    }

    // determine the file size
    fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
    length = ftell(fp);  // return the value of the current position

                         // allocate a buffer with the indicated number of bytes, plus one
    buffer = new char[length + 1];

    // read the appropriate number of bytes from the file
    fseek(fp, 0, SEEK_SET);  // move position indicator to the start of the file
    fread(buffer, 1, length, fp); // read all of the bytes

                                  // append a NULL character to indicate the end of the string
    buffer[length] = '\0';

    // close the file
    fclose(fp);

    // return the string
    return buffer;
}

// Create a GLSL program object from vertex and fragment shader files
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName)
{
    GLuint vertex_shader, fragment_shader;
    GLchar *vs_text, *fs_text;
    GLuint program;

    // check GLSL version
    printf("GLSL version: %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Create shader handlers
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    // Read source code from shader files
    vs_text = readShaderSource(vShaderFileName);
    fs_text = readShaderSource(fShaderFileName);

    // error check
    if (vs_text == NULL) {
        printf("Failed to read from vertex shader file %s\n", vShaderFileName);
        exit(1);
    }
    else if (DEBUG_ON) {
        printf("Vertex Shader:\n=====================\n");
        printf("%s\n", vs_text);
        printf("=====================\n\n");
    }
    if (fs_text == NULL) {
        printf("Failed to read from fragent shader file %s\n", fShaderFileName);
        exit(1);
    }
    else if (DEBUG_ON) {
        printf("\nFragment Shader:\n=====================\n");
        printf("%s\n", fs_text);
        printf("=====================\n\n");
    }

    // Load Vertex Shader
    const char *vv = vs_text;
    glShaderSource(vertex_shader, 1, &vv, NULL);  //Read source
    glCompileShader(vertex_shader); // Compile shaders

                                    // Check for errors
    GLint  compiled;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        printf("Vertex shader failed to compile:\n");
        if (DEBUG_ON) {
            GLint logMaxSize, logLength;
            glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
            printf("printing error message of %d bytes\n", logMaxSize);
            char* logMsg = new char[logMaxSize];
            glGetShaderInfoLog(vertex_shader, logMaxSize, &logLength, logMsg);
            printf("%d bytes retrieved\n", logLength);
            printf("error message: %s\n", logMsg);
            delete[] logMsg;
        }
        exit(1);
    }

    // Load Fragment Shader
    const char *ff = fs_text;
    glShaderSource(fragment_shader, 1, &ff, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);

    //Check for Errors
    if (!compiled) {
        printf("Fragment shader failed to compile\n");
        if (DEBUG_ON) {
            GLint logMaxSize, logLength;
            glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
            printf("printing error message of %d bytes\n", logMaxSize);
            char* logMsg = new char[logMaxSize];
            glGetShaderInfoLog(fragment_shader, logMaxSize, &logLength, logMsg);
            printf("%d bytes retrieved\n", logLength);
            printf("error message: %s\n", logMsg);
            delete[] logMsg;
        }
        exit(1);
    }

    // Create the program
    program = glCreateProgram();

    // Attach shaders to program
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    // Link and set program to use
    glLinkProgram(program);
    glUseProgram(program);

    return program;
}

void Win2PPM(int width, int height) {
    char outdir[10] = "out/"; //Must be defined!
    int i, j;
    FILE* fptr;
    static int counter = 0;
    char fname[32];
    unsigned char *image;

    /* Allocate our buffer for the image */
    image = (unsigned char *)malloc(3 * width*height * sizeof(char));
    if (image == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate memory for image\n");
    }

    /* Open the file */
    sprintf(fname, "%simage_%04d.ppm", outdir, counter);
    if ((fptr = fopen(fname, "w")) == NULL) {
        fprintf(stderr, "ERROR: Failed to open file for window capture\n");
    }

    /* Copy the image into our buffer */
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);

    /* Write the PPM file */
    fprintf(fptr, "P6\n%d %d\n255\n", width, height);
    for (j = height - 1; j >= 0; j--) {
        for (i = 0; i<width; i++) {
            fputc(image[3 * j*width + 3 * i + 0], fptr);
            fputc(image[3 * j*width + 3 * i + 1], fptr);
            fputc(image[3 * j*width + 3 * i + 2], fptr);
        }
    }

    free(image);
    fclose(fptr);
    counter++;
}
