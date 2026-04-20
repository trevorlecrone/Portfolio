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

#define RED_BOMB 0
#define BUTTON 1
#define CRATE 2
#define BLUE_BOMB 3
#define STARS 4
#define NIGHT_SKY 5
#define GAMEBOY 6

bool saveOutput = false;
float timePast = 0;

//load the level parser, and initialize some important data
//just a series of varios globals used throughout the game.
LevelParser l = LevelParser();
string levelArray[4] = { "resources/levels/level1.txt", "resources/levels/level2.txt", "resources/levels/level3.txt", "resources/levels/level4.txt"};
//third peerson level array, currently unimplemented
string tLevelArray[3] = { "resources/levels/level1_t.txt", "resources/levels/level2_t.txt", "resources/levels/level3_t.txt"};
bool heldKeys[5] = { false, false, false, false, false };
glm::vec3 colors[5] = { glm::vec3(1.0f, 0.5f, 1.0f), glm::vec3(0.0f, 0.4f, 1.0f), glm::vec3(1.0f, 0.3f, 0.0f), glm::vec3(0.7f, 1.0f, 1.0f), glm::vec3(0.3f, 0.3f, 0.3f) };
float door_z[5] = { 0.125, 0.125, 0.125, 0.125 };
string keys = "abcde";
string doors = "ABCDE";

//camera first or third person
bool firstPerson = true;
float scale = 1.0f;
bool levelComplete = false;

//camera and direction coordinates
Vec cam = Vec();
Vec atP = Vec(0.0, 0.0, 0.0);
Vec atV = (atP - cam).Normalize();
Vec up = Vec(0.0, 0.0, 1.0);
Vec rght = atV % up;
//last safe camerea position, and the collision camera
Vec sCam = Vec();
Vec cCam = Vec();
//floor transformations
float f_x_scale;
float f_y_scale;
float g_scale;

// global vars for time scaling
//      delta time is used to scale all frames
Uint32 deltaTime = 0;
Uint32 lastTime = 0;

class BombClass {
public:
    bool has = false;
    bool placed = false;
    Uint32 placedTime;
    Uint32 explodeTime;
    int m_x, m_y;
    float x, y;
    void place(float x_, float y_, int width, int height, Uint32 currentTime){
        if(has == true){
            x = x_;
            y = y_;
            m_x = floor((height - x_) / 2);
            m_y = floor((width - y_) / 2);
            cout << m_x << "," << m_y;
            this->has = false;
            this->placed = true;
            placedTime = currentTime;
            explodeTime = currentTime + 2000;
            printf("placed: %u\n", placedTime);
        } else {
            printf("No bomb in inventory.\n");
        }
    }
    void pickup(){
        this->has = true;
    }
    void update(Uint32 currentTime, string* &matrix){
        if(currentTime > explodeTime) explode(matrix);
    }
    void explode(string* &matrix){
        // check walls around the 
        int height = matrix->length();
        int width = matrix[0].length();
        for (int i = -1; i < 2; ++i)
        {
            if(!(m_x+i < 0 || m_x+i >= width)){
                if(matrix[m_x+i][m_y] == 'w') matrix[m_x+i][m_y] = '0';
            }
            if(!(m_y+i < 0 || m_y+i >= height)){
                if(matrix[m_x][m_y+i] == 'w') matrix[m_x][m_y+i] = '0';
            }
        }
        this->placed = false;
    }
};

BombClass bomb = BombClass();
//load textures in to texture slots
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
        if (matrix[x_c][y_c] == 'W' || matrix[x_c][y_c] == '+'|| matrix[x_c][y_c] == 'w') {
            return 0;
        }
        else if (matrix[x_c][y_c] == 'G') { // level goal
            return 2;
        }
		else if (matrix[x_c][y_c] == 'x') { // crate move
			return 3;
		}
        else if (matrix[x_c][y_c] == '*') { // bomb pickup
            return 4;
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

bool check_switches(int width, int height, string* &levelMatrix, string* &originalMatrix) {
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			if (originalMatrix[j][i] == 's') {
				if (levelMatrix[j][i] != 'x') {
					return false;
				}
			}
		}
	}
	return true;
}

//calls parse level, initializes the height and width, and saves the level array for use
string* reinitialize_level(const char* filename, int &width, int &height, int &type, float &h_width, float &h_height) {
    string* lMatrix;
    lMatrix = l.parse(filename, width, height, type);
    //coords useful for transforms
    h_width = floor(((float)width) / 2.0f);
    h_height = floor(((float)height) / 2.0f);

    printf("Width: %d  Height: %d \n", width, height);
    for (int i = 0; i < height; i++) {
        //need to use cout since this is a c++ style string
        cout << lMatrix[i] << endl;
    }
    float player_x{}, player_y{};

    float scale_factor = 1.0f;
    printf("Type %d",type);
    if (type == 1) {
        firstPerson = true;
        scale_factor = 2.0f;
    }
    else {
        firstPerson = false;
    }

    //computes where to place the player in world coordinates
    for (int i = 0; i < height; i++) {
        int s_coord = lMatrix[i].find_first_of('S');
        if (s_coord != -1) {
            player_x = (i - (h_height) == 0) ? 0.0f : -scale_factor * (i - (h_height));
            player_y = (s_coord - (h_width) == 0) ? 0.0f : -scale_factor * (s_coord - (h_width));
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
    f_x_scale = (height * scale_factor) + (2.0f * scale_factor);
    f_y_scale = (width * scale_factor) + (2.0f * scale_factor);
    g_scale = scale_factor;
    return lMatrix;
}

string* initialize_level(const char* filename, int &width, int &height, int &type, float &h_width, float &h_height) {
    string* lMatrix;
    lMatrix = l.parse(filename, width, height, type);
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

    float scale_factor = 1.0f;
    printf("Type %d", type);
    if (type == 1) {
        firstPerson = true;
        scale_factor = 2.0f;
    }
    else {
        firstPerson = false;
    }

    //computes where to place the player in world coordinates
    for (int i = 0; i < height; i++) {
        int s_coord = lMatrix[i].find_first_of('S');
        if (s_coord != -1) {
            player_x = (i - (h_height) == 0) ? 0.0f : -scale_factor * (i - (h_height));
            player_y = (s_coord - (h_width) == 0) ? 0.0f : -scale_factor * (s_coord - (h_width));
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
    f_x_scale = (height * scale_factor) + (2.0f * scale_factor);
    f_y_scale = (width * scale_factor) + (2.0f * scale_factor);
    g_scale = scale_factor;
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

    //parse the level file and store important data
    int width, height;
    int level_index = 0;
    int f_or_t;
    float half_w, half_h;
    string * levelMatrix;
	string * originalMatrix;
    string * tempMatrix;
    float temp_x, temp_y;
    //get the level from the level matrix and convert it into a C style (null terminated) string
    string levelString = levelArray[level_index];
    const char* levelFile = levelString.c_str();
    //perform the parsing of the level file
    levelMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
    tempMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
	originalMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);

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

    //load model 4
    modelFile.open("resources/models/bomb.txt");
    numLines = 0;
    modelFile >> numLines;
    float* model4 = new float[numLines];
    for (int i = 0; i < numLines; i++) {
        modelFile >> model4[i];
    }
    printf("%d\n", numLines);
    int numTris4 = numLines / 8;
    modelFile.close();

    //load model 5
    modelFile.open("resources/models/button.txt");
    numLines = 0;
    modelFile >> numLines;
    float* model5 = new float[numLines];
    for (int i = 0; i < numLines; i++) {
        modelFile >> model5[i];
    }
    printf("%d\n", numLines);
    int numTris5 = numLines / 8;
    modelFile.close();

    //load model 5
    modelFile.open("resources/models/GB.txt");
    numLines = 0;
    modelFile >> numLines;
    float* model6 = new float[numLines];
    for (int i = 0; i < numLines; i++) {
        modelFile >> model6[i];
    }
    printf("%d\n", numLines);
    int numTris6 = numLines / 8;
    modelFile.close();

    //load models in the same way that it was done in the provided source
    float* modelData = new float[(numTris1 + numTris2 + numTris3 + numTris4 + numTris5 + numTris6) * 8];
    copy(model1, model1 + numTris1 * 8, modelData);
    copy(model2, model2 + numTris2 * 8, modelData + numTris1 * 8);
    copy(model3, model3 + numTris3 * 8, modelData + (numTris1 + numTris2) * 8);
    copy(model4, model4 + numTris4 * 8, modelData + (numTris1 + numTris2 + numTris3) * 8);
    copy(model5, model5 + numTris5 * 8, modelData + (numTris1 + numTris2 + numTris3 + numTris4) * 8);
    copy(model6, model6 + numTris6 * 8, modelData + (numTris1 + numTris2 + numTris3 + numTris4 + numTris5) * 8);
    int totalNumTris = numTris1 + numTris2 + numTris3 + numTris4 + numTris5 + numTris6;

    //initialize the textures, array can be made larger
    GLuint misc_Textures[7];
    misc_Textures[RED_BOMB] = loadTexture("resources/textures/red_bomb.bmp");
    misc_Textures[BUTTON] = loadTexture("resources/textures/button_tex.bmp");
    misc_Textures[CRATE] = loadTexture("resources/textures/crate_texture.bmp");
    misc_Textures[BLUE_BOMB] = loadTexture("resources/textures/blue_bomb.bmp");
    misc_Textures[STARS] = loadTexture("resources/textures/stars.bmp");
    misc_Textures[NIGHT_SKY] = loadTexture("resources/textures/night_sky.bmp");
    misc_Textures[GAMEBOY] = loadTexture("resources/textures/gameboy_texture.bmp");

    GLuint floor_Textures[4];
    floor_Textures[0] = loadTexture("resources/textures/tile.bmp");
    floor_Textures[1] = loadTexture("resources/textures/red_tile.bmp");
    floor_Textures[2] = loadTexture("resources/textures/blue_tile.bmp");
    floor_Textures[3] = loadTexture("resources/textures/tile.bmp");

    GLuint wall_Textures[4];
    wall_Textures[0] = loadTexture("resources/textures/brick.bmp");
    wall_Textures[1] = loadTexture("resources/textures/metal.bmp");
    wall_Textures[2] = loadTexture("resources/textures/moon.bmp");
    wall_Textures[3] = loadTexture("resources/textures/youwin.bmp");

    

    GLuint cracked_Textures[3];
    cracked_Textures[0] = loadTexture("resources/textures/cracked_brick.bmp");
    cracked_Textures[1] = loadTexture("resources/textures/cracked_metal.bmp");
    cracked_Textures[2] = loadTexture("resources/textures/cracked_moon.bmp");

    //Allocate memory on the graphics card to store geometry (vertex buffer object)
    GLuint vbo[1];
    glGenBuffers(1, vbo);  //Create 1 buffer called vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
                                           //GL_STATIC_DRAW means we won't change the geometry, GL_DYNAMIC_DRAW = geometry changes infrequently
                                           //GL_STREAM_DRAW = geom. changes frequently.  This effects which types of GPU memory is used

    int shaderProgram = InitShader("shaders/vertexTex.glsl", "shaders/fragmentTex.glsl");
    glUseProgram(shaderProgram); //Set the active shader (only one can be used at a time)



    glEnable(GL_DEPTH_TEST);

    //Event Loop (Loop forever processing each event as fast as possible)
    SDL_Event windowEvent;
    while (true) {
        // calculate time since last frame
        Uint32 currentTime = SDL_GetTicks();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        // NOTE: this approach does not acccount for timer rollover, which in SDL is about 49 days of uptime
        // but we just need to scale every frame update amount by deltaTime to normalize across platforms
        float moveSpeed = 0.004 * deltaTime;

        glm::vec3 glmAt = glm::vec3(atV.X(), atV.Y(), atV.Z());
        if (SDL_PollEvent(&windowEvent)) {
            if (windowEvent.type == SDL_QUIT) {
                SDL_ShowCursor(SDL_ENABLE);
                break;
            }
            //List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
            //Scancode referes to a keyboard position, keycode referes to the letter (e.g., EU keyboards)

            //level warp and texture change checks.
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
                levelMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
				originalMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_2) {
                level_index = 1;
                levelString = levelArray[level_index];
                levelFile = levelString.c_str();
                levelMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
				originalMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_3) {
                level_index = 2;
                levelString = levelArray[level_index];
                levelFile = levelString.c_str();
                levelMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
				originalMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_4) {
                level_index = 3;
                levelString = levelArray[level_index];
                levelFile = levelString.c_str();
                levelMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
				originalMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
            }
            if (!firstPerson) {
                if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_r) {
					levelString = tLevelArray[level_index];
					levelFile = levelString.c_str();
					levelMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
                }
                if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_w) {
                    sCam = cam;
                    cCam = cam + Vec(1.0, 0.0, 0.0);
                    cam = cam + Vec(1.0, 0.0, 0.0);
                    int m_x = floor((half_h - cCam.X()) / g_scale);
                    int m_y = floor((half_w - cCam.Y()) / g_scale);
                    int cCheck = collision_check(m_x, m_y, width, height, levelMatrix);
                    if (cCheck == 0) {
                        cam = sCam;
                    }
                    if (cCheck == 2) {
                        level_index++;
                        levelString = levelArray[level_index];
                        levelFile = levelString.c_str();
                        levelMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
						originalMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
                    }
					if (cCheck == 3) {
						if (collision_check(m_x - 1, m_y, width, height, levelMatrix) == 1 &&
							collision_check(m_x - 1, m_y, width, height, levelMatrix) != 3) {
							levelMatrix[m_x - 1][m_y] = 'x';
							if (originalMatrix[m_x][m_y] == 's') {
								levelMatrix[m_x][m_y] = 's';
							}
							else {
								levelMatrix[m_x][m_y] = '0';
							}
						}
						else {
							cam = sCam;
						}
					}
                }
                if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_a) {
                    sCam = cam;
                    cCam = cam + Vec(0.0, 1.0, 0.0);
                    cam = cam + Vec(0.0, 1.0, 0.0);
                    int m_x = floor((half_h - cCam.X()) / g_scale);
                    int m_y = floor((half_w - cCam.Y()) / g_scale);
                    int cCheck = collision_check(m_x, m_y, width, height, levelMatrix);
                    if (cCheck == 0) {
                        cam = sCam;
                    }
                    if (cCheck == 2) {
                        level_index++;
                        levelString = levelArray[level_index];
                        levelFile = levelString.c_str();
                        levelMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
						originalMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
                    }
					if (cCheck == 3) {
						if (collision_check(m_x, m_y-1, width, height, levelMatrix) == 1 &&
							collision_check(m_x, m_y - 1, width, height, levelMatrix) != 3) {
							levelMatrix[m_x][m_y-1] = 'x';
							if (originalMatrix[m_x][m_y] == 's') {
								levelMatrix[m_x][m_y] = 's';
							}
							else {
								levelMatrix[m_x][m_y] = '0';
							}
						}
						else {
                            cam = sCam;
                        }
                    }
                }
                if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_s) {
                    sCam = cam;
                    cCam = cam - Vec(1.0, 0.0, 0.0);
                    cam = cam - Vec(1.0, 0.0, 0.0);
                    int m_x = floor((half_h - cCam.X()) / g_scale);
                    int m_y = floor((half_w - cCam.Y()) / g_scale);
                    int cCheck = collision_check(m_x, m_y, width, height, levelMatrix);
                    if (cCheck == 0) {
                        cam = sCam;
                    }
                    if (cCheck == 2) {
                        level_index++;
                        levelString = levelArray[level_index];
                        levelFile = levelString.c_str();
                        levelMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
                        originalMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
                    }
                    if (cCheck == 3) {
                        if (collision_check(m_x + 1, m_y, width, height, levelMatrix) == 1 &&
                            collision_check(m_x + 1, m_y, width, height, levelMatrix) != 3) {
                            levelMatrix[m_x + 1][m_y] = 'x';
                            if (originalMatrix[m_x][m_y] == 's') {
                                levelMatrix[m_x][m_y] = 's';
                            }
                            else {
                                levelMatrix[m_x][m_y] = '0';
                            }
                        }
                        else {
                            cam = sCam;
                        }
                    }
                }
                if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_d) {
                    sCam = cam;
                    cCam = cam - Vec(0.0, 1.0, 0.0);
                    cam = cam - Vec(0.0, 1.0, 0.0);
                    int m_x = floor((half_h - cCam.X()) / g_scale);
                    int m_y = floor((half_w - cCam.Y()) / g_scale);
                    int cCheck = collision_check(m_x, m_y, width, height, levelMatrix);
                    if (cCheck == 0) {
                        cam = sCam;
                    }
                    if (cCheck == 2) {
                        level_index++;
                        levelString = levelArray[level_index];
                        levelFile = levelString.c_str();
                        levelMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
                        originalMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
                    }
                    if (cCheck == 3) {
                        if (collision_check(m_x, m_y + 1, width, height, levelMatrix) == 1 &&
                            collision_check(m_x, m_y + 1, width, height, levelMatrix) != 3) {
                            levelMatrix[m_x][m_y + 1] = 'x';
                            if (originalMatrix[m_x][m_y] == 's') {
                                levelMatrix[m_x][m_y] = 's';
                            }
                            else {
                                levelMatrix[m_x][m_y] = '0';
                            }
                        }
                        else {
                            cam = sCam;
                        }
                    }
                }
                if (check_switches(width, height, levelMatrix, originalMatrix)) {
                    levelString = levelArray[level_index];
                    levelFile = levelString.c_str();
                    levelMatrix = reinitialize_level(levelFile, width, height, f_or_t, half_w, half_h);
                    levelMatrix = tempMatrix;
                    for (int i = 0; i < height; i++) {
                        int s_coord = levelMatrix[i].find_first_of('+');
                        if (s_coord != -1) {
                            levelMatrix[i][s_coord] = '*';
                        }
                    }
                    cam = Vec(temp_x, temp_y, cam.Z());
                }
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
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_e) {
                // contextual button
                // if we have the bomb, place it on the current tile
                bomb.place(cam.x, cam.y, width, height, currentTime);
            }

            if (windowEvent.type == SDL_MOUSEBUTTONDOWN && windowEvent.button.button == SDL_BUTTON_LEFT) {
                // contextual button
                // if we have the bomb, place it on the current tile
                int m_x = floor((height - cam.X()) / 2);
                int m_y = floor((width - cam.Y()) / 2);
                cout << round(m_x - atV.X()) << "," << round(m_y + atV.Y()) << endl;

                if ((round(m_x - atV.X()) >= 0) && (round(m_y + atV.Y()) >= 0) && (round(m_x - atV.X()) < height) && (round(m_y + atV.Y()) < width)) {
                    int check_x = round(m_x - atV.X());
                    int check_y = round(m_y + atV.Y());
                    cout << check_x << "," << check_y << endl;
                    if (levelMatrix[check_x][check_y] == '+') {
                        temp_x = cam.X();
                        temp_y = cam.Y();
                        tempMatrix = levelMatrix;
                        levelString = tLevelArray[level_index];
                        levelFile = levelString.c_str();
                        levelMatrix = reinitialize_level(levelFile, width, height, f_or_t, half_w, half_h);
                        originalMatrix = reinitialize_level(levelFile, width, height, f_or_t, half_w, half_h);
                    }
                }
            }
        }

        const Uint8* pressedKeys = SDL_GetKeyboardState(NULL);
        //movement checks
        if (firstPerson) {
            if (pressedKeys[SDL_SCANCODE_W]) { //If "w" is down
                sCam = cam;
                cCam = cam + atV * 2 * moveSpeed;
                cam = cam + atV * moveSpeed;
                int m_x = floor((height - cCam.X()) / 2);
                int m_y = floor((width - cCam.Y()) / 2);
                int cCheck = collision_check(m_x, m_y, width, height, levelMatrix);
                if (cCheck == 0) {
                    cam = sCam;
                }
                if (cCheck == 2) {
                    level_index++;
                    levelString = levelArray[level_index];
                    levelFile = levelString.c_str();
                    levelMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
					originalMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
                }
                if (cCheck == 4) {
                    bomb.has = true;
                }
            }
            if (pressedKeys[SDL_SCANCODE_S]) { //If "s" is down
                sCam = cam;
                cCam = cam - atV * 2 * moveSpeed;
                cam = cam - atV * moveSpeed;
                int m_x = floor((height - cCam.X()) / 2);
                int m_y = floor((width - cCam.Y()) / 2);
                int cCheck = collision_check(m_x, m_y, width, height, levelMatrix);
                if (cCheck == 0) {
                    cam = sCam;
                }
                if (cCheck == 2) {
                    level_index++;
                    levelString = levelArray[level_index];
                    levelFile = levelString.c_str();
                    levelMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
					originalMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
                }
                if (cCheck == 4) {
                    bomb.has = true;
                }
            }
            if (pressedKeys[SDL_SCANCODE_A]) { //If "a" is down
                sCam = cam;
                cCam = cam - rght * 2 * moveSpeed;
                cam = cam - rght * moveSpeed;
                int m_x = floor((height - cCam.X()) / 2);
                int m_y = floor((width - cCam.Y()) / 2);
                int cCheck = collision_check(m_x, m_y, width, height, levelMatrix);
                if (cCheck == 0) {
                    cam = sCam;
                }
                if (cCheck == 2) {
                    level_index++;
                    levelString = levelArray[level_index];
                    levelFile = levelString.c_str();
                    levelMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
					originalMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
                }
                if (cCheck == 4) {
                    bomb.has = true;
                }
            }
            if (pressedKeys[SDL_SCANCODE_D]) { //If "d" is down
                sCam = cam;
                cCam = cam + rght * 2 * moveSpeed;
                cam = cam + rght * moveSpeed;
                int m_x = floor((height - cCam.X()) / 2);
                int m_y = floor((width - cCam.Y()) / 2);
                int cCheck = collision_check(m_x, m_y, width, height, levelMatrix);
                if (cCheck == 0) {
                    cam = sCam;
                }
                if (cCheck == 2) {
                    level_index++;
                    levelString = levelArray[level_index];
                    levelFile = levelString.c_str();
                    levelMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
					originalMatrix = initialize_level(levelFile, width, height, f_or_t, half_w, half_h);
                }
                if (cCheck == 4) {
                    bomb.has = true;
                }
            }
            if (pressedKeys[SDL_SCANCODE_LEFT]) { //If the left arrow is down
                glmAt = glm::rotateZ(glmAt, moveSpeed / 2);
                atV = Vec(glmAt.x, glmAt.y, glmAt.z);
                rght = atV % up;
            }
            if (pressedKeys[SDL_SCANCODE_RIGHT]) { //If the right arrow is down
                glmAt = glm::rotateZ(glmAt, -moveSpeed / 2);
                atV = Vec(glmAt.x, glmAt.y, glmAt.z);
                rght = atV % up;
            }
        }
        if (!saveOutput) timePast = SDL_GetTicks() / 1000.f;
        if (saveOutput) timePast += .07; //Fix framerate at 14 FPS

        //Some default openGL stuff
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
        GLint uniScaleTex = glGetUniformLocation(shaderProgram, "scaleTex");
        GLint uniInColor = glGetUniformLocation(shaderProgram, "inColor");

        // Clear the screen to default color
        glClearColor(.7f, 0.7f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // first person camera
        if (firstPerson) {
            glm::mat4 view = glm::lookAt(
                glm::vec3(cam.X(), cam.Y(), cam.Z()),  //Cam Position
                glm::vec3(cam.X() + atV.X(), cam.Y() + atV.Y(), cam.Z() + atV.Z()),  //Look at point
                glm::vec3(0.0f, 0.0f, 1.0f)); //Up

            glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
        }
        // third person camera
        else {
            glm::mat4 view = glm::lookAt(
                glm::vec3(cam.X() + 0.1, cam.Y(), cam.Z() + 15),  //Cam Position
                glm::vec3(cam.X(), cam.Y(), cam.Z()),  //Look at point
                glm::vec3(1.0f, 0.0f, 0.0f)); //Up

            glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
        }

        glm::mat4 proj = glm::perspective(3.14f / 4, 800.0f / 600.0f, 0.025f, 30.0f); //FOV, aspect, near, far
        glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
        glBindTexture(GL_TEXTURE_2D, floor_Textures[level_index]);
        glm::mat4 model;
        model = glm::scale(model, glm::vec3(f_x_scale, f_y_scale, 0.5f));
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.0f));
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(uniUseTex, 1);
        glUniform1i(uniScaleTex, 1);
        glDrawArrays(GL_TRIANGLES, 0, numTris1);

        glUniform1i(uniScaleTex, 0);
        //loop to draw the walls, doors, keys, bombs, crates, switches, etc.
        //starts at -1 and ends at height/width to enclose the level in walls
        for (int i = -1; i <= height; i++) {
            for (int j = -1; j <= width; j++) {
                //draw the outer ring of the level full of walls, and any square in the level which is a W
                if (i < 0 || j < 0 || i == height || j == width || levelMatrix[i][j] == 'W') {
                    float trans_x = -g_scale * (i - half_h);
                    float trans_y = -g_scale * (j - half_w);
                    float trans_z = g_scale == 1 ? -0.25f : 0.125f;
                    model = glm::mat4();
                    model = glm::translate(model, glm::vec3(trans_x, trans_y, trans_z));
                    model = glm::scale(model, glm::vec3(g_scale, g_scale, g_scale));
				    glBindTexture(GL_TEXTURE_2D, wall_Textures[level_index]);
                    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
                    glUniform1i(uniUseTex, 1);
                    glDrawArrays(GL_TRIANGLES, 0, numTris1);
                }
                else {
                    int key = keys.find(levelMatrix[i][j]);
                    int door = doors.find(levelMatrix[i][j]);
                    //if the index we are looking at in the matrix represents a key
                    if (levelMatrix[i][j] == 'w') {
                        float trans_x = -g_scale * (i - half_h);
                        float trans_y = -g_scale * (j - half_w);
                        model = glm::mat4();
                        model = glm::translate(model, glm::vec3(trans_x, trans_y, 0.125f));
                        model = glm::scale(model, glm::vec3(g_scale, g_scale, g_scale));
						glBindTexture(GL_TEXTURE_2D, cracked_Textures[level_index]);
                        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform1i(uniUseTex, 1);
                        glDrawArrays(GL_TRIANGLES, 0, numTris1);
                    }

                    if (key >= 0) {
                        float trans_x = -g_scale * (i - half_h);
                        float trans_y = -g_scale * (j - half_w);
                        model = glm::mat4();
                        model = glm::translate(model, glm::vec3(trans_x, trans_y, (0.1f *sinf(1.5f * timePast)) - 0.05f));
                        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
                        model = glm::rotate(model, 3.14159f / 2, glm::vec3(1.0f, 0.0f, 0.0f));
                        //use  the index of the key to color it
                        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform3fv(uniInColor, 1, glm::value_ptr(colors[key]));
                        glUniform1i(uniUseTex, 0);
                        glDrawArrays(GL_TRIANGLES, numTris1, numTris2);
                    }
                    // if the index we are looking at is a door
                    if (door >= 0) {
                        float trans_x = -g_scale * (i - half_h);
                        float trans_y = -g_scale * (j - half_w);
                        float trans_z = 0.125;
                        //animate the door opening if the corresponding key is held
                        if (heldKeys[door]) {
                            if (door_z[door] <= 2.125) {
                                door_z[door] = door_z[door] + 0.01;
                            }
                            trans_z = door_z[door];
                        }
                        model = glm::mat4();
                        model = glm::translate(model, glm::vec3(trans_x, trans_y, trans_z));
                        model = glm::scale(model, glm::vec3(g_scale, g_scale, g_scale));
                        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
                        //use  the index to color the door.
                        glUniform3fv(uniInColor, 1, glm::value_ptr(colors[door]));
                        glUniform1i(uniUseTex, 0);
                        glDrawArrays(GL_TRIANGLES, 0, numTris1);
                    }
                    // Goal
                    if (levelMatrix[i][j] == 'G') {
                        float trans_x = -g_scale * (i - half_h);
                        float trans_y = -g_scale * (j - half_w);
                        model = glm::mat4();
                        model = glm::translate(model, glm::vec3(trans_x, trans_y, (0.1f *sinf(1.5f * timePast)) - 0.05f));
                        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
                        model = glm::rotate(model, 3.14159f / 2, glm::vec3(1.0f, 0.0f, 0.0f));
                        model = glm::rotate(model, timePast * .6f * 3.14f / 4, glm::vec3(0.0f, 1.0f, 0.0f));
                        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
                        //cycle through colors
                        glUniform3fv(uniInColor, 1, glm::value_ptr(glm::vec3(0.5 + (sin(timePast * 0.1) / 2), 0.5 + (sin(timePast * 0.3) / 2), 0.5 + (sin(timePast * 0.15) / 2))));
                        glUniform1i(uniUseTex, 0);
                        glDrawArrays(GL_TRIANGLES, numTris1 + numTris2, numTris3);
                    }
                    // Blue bomb (pickup)
                    if (levelMatrix[i][j] == '*') {
                        if(!bomb.has){
                            float trans_x = -g_scale * (i - half_h);
                            float trans_y = -g_scale * (j - half_w);
                            model = glm::mat4();
                            model = glm::translate(model, glm::vec3(trans_x, trans_y, -0.2f));
                            model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
                            model = glm::rotate(model, 3.14159f / 2, glm::vec3(1.0f, 0.0f, 0.0f));
                            model = glm::rotate(model, timePast * .6f * 3.14f / 4, glm::vec3(0.0f, 1.0f, 0.0f));
                            glBindTexture(GL_TEXTURE_2D, misc_Textures[BLUE_BOMB]);
                            glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
                            glUniform1i(uniUseTex, 1);
                            glDrawArrays(GL_TRIANGLES, numTris1 + numTris2 + numTris3, numTris4);
                        }
                    }
                    // Placed bomb
                    if (bomb.placed) {
                        float x_scale_factor = sin(currentTime / 100.0f) * 0.25;
                        float y_scale_factor = sin(currentTime / 100.0f + 0.535) * 0.3;

                        model = glm::mat4();
                        bomb.update(currentTime, levelMatrix);
                        model = glm::translate(model, glm::vec3(bomb.x, bomb.y, -0.2f));
                        // model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
                        model = glm::scale(model, glm::vec3(1.0f + x_scale_factor, 1.0f + x_scale_factor, 1.0f + y_scale_factor));
                        model = glm::rotate(model, 3.14159f / 2, glm::vec3(1.0f, 0.0f, 0.0f));
                        if (x_scale_factor >= 0) {
                            glBindTexture(GL_TEXTURE_2D, misc_Textures[BLUE_BOMB]);
                        }
                        else {
                            glBindTexture(GL_TEXTURE_2D, misc_Textures[RED_BOMB]);
                        }
                        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform1i(uniUseTex, 1);
                        glDrawArrays(GL_TRIANGLES, numTris1 + numTris2 + numTris3, numTris4);
                    }
                    if (bomb.has) {
                        model = glm::mat4();
                        float key_t_x = cam.X() + (0.03 * atV.X());
                        float key_t_y = cam.Y() + (0.03 * atV.Y());
                        //keeps the bomb at the bottom of the screen.
                        model = glm::translate(model, glm::vec3(key_t_x, key_t_y, cam.Z() - 0.01));
                        model = glm::scale(model, glm::vec3(0.001f, 0.001f, 0.001f));
                        model = glm::rotate(model, 3.14159f / 2, glm::vec3(1.0f, 0.0f, 0.0f));
                        model = glm::rotate(model, timePast * .6f * 3.14f / 4, glm::vec3(0.0f, 1.0f, 0.0f));
                        glBindTexture(GL_TEXTURE_2D, misc_Textures[BLUE_BOMB]);
                        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform1i(uniUseTex, 1);
                        glDrawArrays(GL_TRIANGLES, numTris1 + numTris2 + numTris3, numTris4);
                    }
                    // Red swtich
                    if (levelMatrix[i][j] == 's') {
                        float trans_x = -g_scale * (i - half_h);
                        float trans_y = -g_scale * (j - half_w);
                        model = glm::mat4();
                        model = glm::translate(model, glm::vec3(trans_x, trans_y, -0.72f));
                        model = glm::rotate(model, 3.14159f / 2, glm::vec3(1.0f, 0.0f, 0.0f));
                        glBindTexture(GL_TEXTURE_2D, misc_Textures[BUTTON]);
                        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform1i(uniUseTex, 1);
                        glDrawArrays(GL_TRIANGLES, numTris1 + numTris2 + numTris3 + numTris4, numTris5);
                    }
                    // gameboy
                    if (levelMatrix[i][j] == '+') {
                        float trans_x = -g_scale * (i - half_h);
                        float trans_y = -g_scale * (j - half_w);
                        model = glm::mat4();
                        model = glm::translate(model, glm::vec3(trans_x, trans_y, 0.0f));
                        model = glm::rotate(model, -3.14159f / 2, glm::vec3(1.0f, 0.0f, 0.0f));
                        model = glm::rotate(model, timePast * .6f * 3.14f / 4, glm::vec3(0.0f, 1.0f, 0.0f));
                        glBindTexture(GL_TEXTURE_2D, misc_Textures[GAMEBOY]);
                        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform1i(uniUseTex, 1);
                        glDrawArrays(GL_TRIANGLES, numTris1 + numTris2 + numTris3 + numTris4 + numTris5, numTris6);
                    }
                    // Crate
                    if (levelMatrix[i][j] == 'x') {
                        float trans_x = -g_scale * (i - half_h);
                        float trans_y = -g_scale * (j - half_w);
                        float trans_z = g_scale == 1 ? -0.25f : 0.125f;
                        model = glm::mat4();
                        model = glm::translate(model, glm::vec3(trans_x, trans_y, trans_z));
                        model = glm::scale(model, glm::vec3(g_scale, g_scale, g_scale)); //An example of scale
					    glBindTexture(GL_TEXTURE_2D, misc_Textures[CRATE]);
						glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
						glUniform1i(uniUseTex, 1);
						glDrawArrays(GL_TRIANGLES, 0, numTris1);
                    }
                    //loop to draw the keys in the inventory
                    for (int i = 0; i < 5; i++) {
                        if (heldKeys[i]) {
                            model = glm::mat4();
                            //r_factor determines the screen "x" coordinate of the key, heldKeys[0] is the furthest to the left, heldKeys[2] is dead center, and heldKeys[4] is the far right
                            float r_factor = ((-2 + i) * 0.005);
                            float key_t_x = cam.X() + (0.03 * atV.X()) + (r_factor * rght.X());
                            float key_t_y = cam.Y() + (0.03 * atV.Y()) + (r_factor * rght.Y());
                            //keeps the keys at the bottom of the screen.
                            model = glm::translate(model, glm::vec3(key_t_x, key_t_y, cam.Z() - 0.01));
                            model = glm::scale(model, glm::vec3(0.001f, 0.001f, 0.001f));
                            model = glm::rotate(model, 3.14159f / 2, glm::vec3(1.0f, 0.0f, 0.0f));
                            //probably want to remove this, is expensive to call
                            glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
                            glUniform3fv(uniInColor, 1, glm::value_ptr(colors[i]));
                            glUniform1i(uniUseTex, 0);
                            glDrawArrays(GL_TRIANGLES, numTris1, numTris2);
                        }
                    }
                }
            }
        }

        ///////////////////////////////////////////////////////
        //I did not write any code beyond this point - Trevor//
        ///////////////////////////////////////////////////////

        // Render player (if third person)
        if (!firstPerson) {
            model = glm::mat4();
            model = glm::translate(model, glm::vec3(cam.X(), cam.Y(), 0.125f));
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
            glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(uniInColor, 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
            glUniform1i(uniUseTex, 0);
            glDrawArrays(GL_TRIANGLES, 0, numTris1);
        } else {
            // render skybox
            model = glm::mat4();
            model = glm::translate(model, glm::vec3(cam.X(), cam.Y(), cam.Z()));
            model = glm::scale(model, glm::vec3(30.0, 30.0, 30.0));
            glBindTexture(GL_TEXTURE_2D, misc_Textures[STARS]);
            glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(uniUseTex, 1);
            glDrawArrays(GL_TRIANGLES, 0, numTris1);
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