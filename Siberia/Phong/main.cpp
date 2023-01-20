#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <thread>
#include "shader.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "texture.hpp"

float
Clamp(float x, float min, float max) {
    return x < min ? min : x > max ? max : x;
}

int WindowWidth = 800;
int WindowHeight = 800;
const float TargetFPS = 60.0f;
const std::string WindowTitle = "Siberia";

/*
enum ShadingMode {
    GOURAUD = 0,
    PHONG,
    PHONG_MATERIAL,
    PHONG_MATERIAL_TEXTURE
};
*/

struct Input {
    bool MoveLeft;
    bool MoveRight;
    bool MoveUp;
    bool MoveDown;
    bool LookLeft;
    bool LookRight;
    bool LookUp;
    bool LookDown;
};

static float pos1 = 0;
static float pos2 = 0;
static float pos3 = 0;

struct EngineState {
    Input* mInput;
    Camera* mCamera;
    unsigned mShadingMode;
    bool mDrawDebugLines;
    float mDT;
};


static void
ErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error: " << description << std::endl;
}

static bool pressed = true;

static void
KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    EngineState* State = (EngineState*)glfwGetWindowUserPointer(window);
    Input* UserInput = State->mInput;
    bool IsDown = action == GLFW_PRESS || action == GLFW_REPEAT;
    switch (key) {
    case GLFW_KEY_A: UserInput->MoveLeft = IsDown; break;
    case GLFW_KEY_D: UserInput->MoveRight = IsDown; break;
    case GLFW_KEY_W: UserInput->MoveUp = IsDown; break;
    case GLFW_KEY_S: UserInput->MoveDown = IsDown; break;

    case GLFW_KEY_RIGHT: UserInput->LookLeft = IsDown; break;
    case GLFW_KEY_LEFT: UserInput->LookRight = IsDown; break;
    case GLFW_KEY_UP: UserInput->LookUp = IsDown; break;
    case GLFW_KEY_DOWN: UserInput->LookDown = IsDown; break;

    //case GLFW_KEY_1: State->mShadingMode = 0; break;
    case GLFW_KEY_2: pressed = false; break;
    case GLFW_KEY_3: pressed = true; break;
    //case GLFW_KEY_4: State->mShadingMode = 3; break;

    //case GLFW_KEY_5: pos1 += 0.1; printf("Pozicija1 = %.1f Pozicija2 = % .1f Pozicija3 = %.1f ", pos1, pos2, pos3); break;
    //case GLFW_KEY_6: pos1 -= 0.1; printf("Pozicija1 = %.1f Pozicija2 = % .1f Pozicija3 = %.1f ", pos1, pos2, pos3); break;
    case GLFW_KEY_7: if(pos2<1.5)pos2 += 0.1; printf("Pozicija1 = %.1f Pozicija2 = % .1f Pozicija3 = %.1f ", pos1, pos2, pos3); break;
    case GLFW_KEY_8: if(pos2>-0.5)pos2 -= 0.1; printf("Pozicija1 = %.1f Pozicija2 = % .1f Pozicija3 = %.1f ", pos1, pos2, pos3); break;
    //case GLFW_KEY_9: pos3 += 0.1; printf("Pozicija1 = %.1f Pozicija2 = % .1f Pozicija3 = %.1f ", pos1, pos2, pos3); break;
    //case GLFW_KEY_0: pos3 -= 0.1; printf("Pozicija1 = %.1f Pozicija2 = % .1f Pozicija3 = %.1f ", pos1, pos2, pos3); break;

    case GLFW_KEY_L: {
        if (IsDown) {
            State->mDrawDebugLines ^= true; break;
        }
    } break;

    case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
    }
}

static void
FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    WindowWidth = width;
    WindowHeight = height;
    glViewport(0, 0, width, height);
}

/**
 * @brief Updates engine state based on input
 * 
 * @param state EngineState
 */
static void
HandleInput(EngineState* state) {
    Input* UserInput = state->mInput;
    Camera* FPSCamera = state->mCamera;
    if (UserInput->MoveLeft) FPSCamera->Move(-1.0f, 0.0f, state->mDT);
    if (UserInput->MoveRight) FPSCamera->Move(1.0f, 0.0f, state->mDT);
    if (UserInput->MoveDown) FPSCamera->Move(0.0f, -1.0f, state->mDT);
    if (UserInput->MoveUp) FPSCamera->Move(0.0f, 1.0f, state->mDT);

    if (UserInput->LookLeft) FPSCamera->Rotate(1.0f, 0.0f, state->mDT);
    if (UserInput->LookRight) FPSCamera->Rotate(-1.0f, 0.0f, state->mDT);
    if (UserInput->LookDown) FPSCamera->Rotate(0.0f, -1.0f, state->mDT);
    if (UserInput->LookUp) FPSCamera->Rotate(0.0f, 1.0f, state->mDT);
}

/**
 * @brief Draws flattened cubes
 *
 * @param vao - Cube VAO
 * @param shader - Shader
 */
/*
static void
DrawFloor(unsigned vao, const Shader& shader, unsigned diffuse, unsigned specular) {
    glUseProgram(shader.GetId());
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specular);
    float Size = 4.0f;
    for (int i = -2; i < 4; ++i) {
        for (int j = -2; j < 4; ++j) {
            glm::mat4 Model(1.0f);
            Model = glm::translate(Model, glm::vec3(i * Size, -2.0f, j * Size));
            Model = glm::scale(Model, glm::vec3(Size, 0.1f, Size));
            shader.SetModel(Model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    glBindVertexArray(0);
    glUseProgram(0);
}
*/
int main() {
    GLFWwindow* Window = 0;
    if (!glfwInit()) {
        std::cerr << "Failed to init glfw" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    Window = glfwCreateWindow(WindowWidth, WindowHeight, WindowTitle.c_str(), 0, 0);
    if (!Window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(Window);

    GLenum GlewError = glewInit();
    if (GlewError != GLEW_OK) {
        std::cerr << "Failed to init glew: " << glewGetErrorString(GlewError) << std::endl;
        glfwTerminate();
        return -1;
    }

    EngineState State = { 0 };
    Camera FPSCamera;
    Input UserInput = { 0 };
    State.mCamera = &FPSCamera;
    State.mInput = &UserInput;
    glfwSetWindowUserPointer(Window, &State);

    glfwSetErrorCallback(ErrorCallback);
    glfwSetFramebufferSizeCallback(Window, FramebufferSizeCallback);
    glfwSetKeyCallback(Window, KeyCallback);

    glViewport(0.0f, 0.0f, WindowWidth, WindowHeight);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    unsigned CubeDiffuseTexture = Texture::LoadImageToTexture("res/container_diffuse.png");
    unsigned CubeSpecularTexture = Texture::LoadImageToTexture("res/container_specular.png");
    unsigned FloorDiffuseTexture = Texture::LoadImageToTexture("res/floor_diffuse.jpg");
    unsigned FloorSpecularTexture = Texture::LoadImageToTexture("res/floor_specular.jpg");
    unsigned IceDiffuseTexture = Texture::LoadImageToTexture("res/ice.jpg");
    unsigned WaterDiffuseTexture = Texture::LoadImageToTexture("res/water.jpg");
    unsigned WaterSpecularTexture = Texture::LoadImageToTexture("res/water1.jpg");
    unsigned IceSpecularTexture = Texture::LoadImageToTexture("res/ice1.jpg");
    unsigned TentDiffuseTexture = Texture::LoadImageToTexture("res/tent.jpg");
    unsigned BearDiffuseTexture = Texture::LoadImageToTexture("res/bear.jpg");

    std::vector<float> CubeVertices = {
        // X     Y     Z     NX    NY    NZ    U     V    FRONT SIDE
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // L D
         0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // L U
         0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // R D
         0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // R U
        -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // L U
                                                        // LEFT SIDE
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // L D
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // R U
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
                                                        // RIGHT SIDE
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // L D
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // R U
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
                                                        // BOTTOM SIDE
                   
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // L D
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // R D
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // L U
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // R D
         0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // R U
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // L U

                                                        // TOP SIDE
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // L D
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // L U
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // R D
         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // R U
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // L U
                                                        // BACK SIDE
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // L D
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // R D
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // L U
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // R U
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // L U
    };

    std::vector<float> TentVertices = {
        // X     Y     Z     NX    NY    NZ    U     V    
        /*
       //FRONT SIDE
        
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // L D
         0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // R D
        0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // L U
        */
        // LEFT SIDE
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // L D
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
         0.0f,  0.5f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U

         // RIGHT SIDE
        0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // L D
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
        0.0f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U

        /*
        // BOTTOM SIDE
        
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // L D
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // R D
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // L U
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // R D
         0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // R U
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // L U
        */
        // BACK SIDE
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // L D
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // R D
         0.0f,  0.5f, -0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // L U

    };

    unsigned CubeVAO;
    glGenVertexArrays(1, &CubeVAO);
    glBindVertexArray(CubeVAO);
    unsigned CubeVBO;
    glGenBuffers(1, &CubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, CubeVBO);
    glBufferData(GL_ARRAY_BUFFER, CubeVertices.size() * sizeof(float), CubeVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);



    unsigned TentVAO;
    glGenVertexArrays(1, &TentVAO);
    glBindVertexArray(TentVAO);
    unsigned TentVBO;
    glGenBuffers(1, &TentVBO);
    glBindBuffer(GL_ARRAY_BUFFER, TentVBO);
    glBufferData(GL_ARRAY_BUFFER, TentVertices.size() * sizeof(float), TentVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);



    Model Bear("res/polar-bear/2020.obj");
    if (!Bear.Load()) {
        std::cerr << "Failed to load bear\n";
        glfwTerminate();
        return -1;
    }

    // NOTE(Jovan): Used to only define color
    Shader ColorShader("shaders/color.vert", "shaders/color.frag");

    //Phong shader with material and texture support
    Shader PhongShaderMaterialTexture("shaders/basic.vert", "shaders/phong_material_texture.frag");
    glUseProgram(PhongShaderMaterialTexture.GetId());

    PhongShaderMaterialTexture.SetUniform3f("uDirLight.Position", glm::vec3(0.0f, 15.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uDirLight.Direction", glm::vec3(1.0f, -1.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uDirLight.Ka", glm::vec3(0.3f, 0.3f, 0.3f));
    PhongShaderMaterialTexture.SetUniform3f("uDirLight.Kd", glm::vec3(0.3f, 0.3f, 0.3f));
    PhongShaderMaterialTexture.SetUniform3f("uDirLight.Ks", glm::vec3(1.0f));

    //lamp light setup
    PhongShaderMaterialTexture.SetUniform3f("uPointLight.Position", glm::vec3(-4.1f, -0.1f, -1.5f));
    PhongShaderMaterialTexture.SetUniform3f("uPointLight.Ks", glm::vec3(1.0f));
    PhongShaderMaterialTexture.SetUniform1f("uPointLight.Kc", 1.0f);
    PhongShaderMaterialTexture.SetUniform1f("uPointLight.Kl", 0.092f);
    PhongShaderMaterialTexture.SetUniform1f("uPointLight.Kq", 0.032f);

    
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[0].Position", glm::vec3(-1.1, 0.0, 0.0));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[0].Direction", glm::vec3(0.0f, -1.0f, 1.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[0].Ka", glm::vec3(0.0f, 0.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[0].Kd", glm::vec3(0.0f, 0.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[0].Ks", glm::vec3(1.0f));
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[0].Kc", 1.0f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[0].Kl", 0.092f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[0].Kq", 0.032f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[0].InnerCutOff", glm::cos(glm::radians(12.5f)));
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[0].OuterCutOff", glm::cos(glm::radians(17.5f)));

    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[1].Position", glm::vec3(-1.1, 0.0, 0.0));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[1].Direction", glm::vec3(0.0f, -1.0f, -1.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[1].Ka", glm::vec3(0.0f, 0.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[1].Kd", glm::vec3(0.0f, 0.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[1].Ks", glm::vec3(1.0f));
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[1].Kc", 1.0f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[1].Kl", 0.092f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[1].Kq", 0.032f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[1].InnerCutOff", glm::cos(glm::radians(12.5f)));
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[1].OuterCutOff", glm::cos(glm::radians(17.5f)));

    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[2].Position", glm::vec3(-1.1, 0.0, 0.0));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[2].Direction", glm::vec3(-1.0f, -1.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[2].Ka", glm::vec3(0.0f, 0.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[2].Kd", glm::vec3(0.0f, 0.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[2].Ks", glm::vec3(1.0f));
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[2].Kc", 1.0f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[2].Kl", 0.092f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[2].Kq", 0.032f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[2].InnerCutOff", glm::cos(glm::radians(12.5f)));
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[2].OuterCutOff", glm::cos(glm::radians(17.5f)));

    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[3].Position", glm::vec3(-1.1, 0.0, 0.0));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[3].Direction", glm::vec3(1.0f, -1.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[3].Ka", glm::vec3(0.0f, 0.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[3].Kd", glm::vec3(0.0f, 0.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[3].Ks", glm::vec3(1.0f));
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[3].Kc", 1.0f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[3].Kl", 0.092f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[3].Kq", 0.032f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[3].InnerCutOff", glm::cos(glm::radians(12.5f)));
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[3].OuterCutOff", glm::cos(glm::radians(17.5f)));

    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[4].Position", glm::vec3(-1.1, 0.0, 0.0));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[4].Direction", glm::vec3(0.0f, -1.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[4].Ka", glm::vec3(0.0f, 0.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[4].Kd", glm::vec3(0.0f, 0.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[4].Ks", glm::vec3(1.0f));
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[4].Kc", 1.0f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[4].Kl", 0.092f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[4].Kq", 0.032f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[4].InnerCutOff", glm::cos(glm::radians(12.5f)));
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[4].OuterCutOff", glm::cos(glm::radians(17.5f)));

    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[5].Position", glm::vec3(-1.1, 0.0, 0.0));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[5].Direction", glm::vec3(0.0f, 1.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[5].Ka", glm::vec3(0.0f, 0.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[5].Kd", glm::vec3(0.0f, 0.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight[5].Ks", glm::vec3(1.0f));
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[5].Kc", 1.0f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[5].Kl", 0.092f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[5].Kq", 0.032f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[5].InnerCutOff", glm::cos(glm::radians(12.5f)));
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight[5].OuterCutOff", glm::cos(glm::radians(17.5f)));

    /*
    //Diminishes the light's diffuse component by half, tinting it slightly red
    PhongShaderMaterialTexture.SetUniform1i("uMaterial.Kd", 0);
    */
    // Makes the object really shiny
    PhongShaderMaterialTexture.SetUniform1i("uMaterial.Ks", 1);
    PhongShaderMaterialTexture.SetUniform1f("uMaterial.Shininess", 128.0f);
    
    glUseProgram(0);

    glm::mat4 Projection = glm::perspective(45.0f, WindowWidth / (float)WindowHeight, 0.1f, 100.0f);
    glm::mat4 View = glm::lookAt(FPSCamera.GetPosition(), FPSCamera.GetTarget(), FPSCamera.GetUp());
    glm::mat4 ModelMatrix(1.0f);
    
    //Current angle around Y axis, with regards to XZ plane at which the point light is situated at
    float Angle = 0.0f;
    //Distance of point light from center of rotation
    float Distance = 5.0f;
    float TargetFrameTime = 1.0f / TargetFPS;
    float StartTime = glfwGetTime();
    float EndTime = glfwGetTime();
    float res = 0.0;
    glClearColor(0.1f, 0.1f, 0.2f, 0.0f);

    //Currently used shader
    Shader* CurrentShader = &PhongShaderMaterialTexture;
    while (!glfwWindowShouldClose(Window)) {
        glfwPollEvents();
        HandleInput(&State);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //In case of window resize, update projection. Bit bad for performance to do it every iteration.
        // If laggy, remove this line
        Projection = glm::perspective(45.0f, WindowWidth / (float)WindowHeight, 0.1f, 100.0f);
        View = glm::lookAt(FPSCamera.GetPosition(), FPSCamera.GetTarget(), FPSCamera.GetUp());
        StartTime = glfwGetTime();
        glUseProgram(CurrentShader->GetId());
        CurrentShader->SetProjection(Projection);
        CurrentShader->SetView(View);
        CurrentShader->SetUniform3f("uViewPos", FPSCamera.GetPosition());

        if (pressed)
            res = 0.5;
        else res = 0.0;

        // NOTE(Jovan): Rotate point light around 0, 0, -2
        glm::vec3 PointLightPosition(Distance * cos(Angle), 2.0f, -2.0f + Distance * sin(Angle));

        //lamp on/off switch 
        CurrentShader->SetUniform3f("uPointLight.Ka", glm::vec3(res, res, 0.0));
        CurrentShader->SetUniform3f("uPointLight.Kd", glm::vec3(res, res, 0.0));

        CurrentShader->SetUniform3f("uSpotlight[0].Position", glm::vec3(-1.1, pos2, 0.0));
        CurrentShader->SetUniform3f("uSpotlight[0].Ka", glm::vec3(pos2, 0.0, 0.0));
        CurrentShader->SetUniform3f("uSpotlight[0].Kd", glm::vec3(pos2, 0.0, 0.0));

        CurrentShader->SetUniform3f("uSpotlight[1].Position", glm::vec3(-1.1, pos2, 0.0));
        CurrentShader->SetUniform3f("uSpotlight[1].Ka", glm::vec3(0.0, pos2, 0.0));
        CurrentShader->SetUniform3f("uSpotlight[1].Kd", glm::vec3(0.0, pos2, 0.0));


        CurrentShader->SetUniform3f("uSpotlight[2].Position", glm::vec3(-1.1, pos2, 0.0));
        CurrentShader->SetUniform3f("uSpotlight[2].Ka", glm::vec3(0.0, 0.0, pos2));
        CurrentShader->SetUniform3f("uSpotlight[2].Kd", glm::vec3(0.0, 0.0, pos2));

        CurrentShader->SetUniform3f("uSpotlight[3].Position", glm::vec3(-1.1, pos2, 0.0));
        CurrentShader->SetUniform3f("uSpotlight[3].Ka", glm::vec3(pos2, pos2, 0.0));
        CurrentShader->SetUniform3f("uSpotlight[3].Kd", glm::vec3(pos2, pos2, 0.0));

        CurrentShader->SetUniform3f("uSpotlight[4].Position", glm::vec3(-1.1, pos2, 0.0));
        CurrentShader->SetUniform3f("uSpotlight[4].Ka", glm::vec3(pos2, 0.0, pos2));
        CurrentShader->SetUniform3f("uSpotlight[4].Kd", glm::vec3(pos2, 0.0, pos2));

        CurrentShader->SetUniform3f("uSpotlight[5].Position", glm::vec3(-1.1, pos2, 0.0));
        CurrentShader->SetUniform3f("uSpotlight[5].Ka", glm::vec3(pos2, pos2, pos2));
        CurrentShader->SetUniform3f("uSpotlight[5].Kd", glm::vec3(pos2, pos2, pos2));


        Angle += State.mDT;

        //Tent
        ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-4.1, 0.6, -1.6));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2, 2, 2));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TentDiffuseTexture);
        glBindVertexArray(TentVAO);
        glDrawArrays(GL_TRIANGLES, 0, TentVertices.size() / 8);

        //Stick
        ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.4, 0.4, 0.2));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(20.0f), glm::vec3(0.0, 0.0, 1.0));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.05, 3.0, 0.05));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, CubeDiffuseTexture);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

        //Water
        ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0, -0.3, -0.2));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0, 0.05, 2.0));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, WaterDiffuseTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, WaterSpecularTexture);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

        //Ice
        ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -0.6, 0.2));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(15.0, 0.2, 10));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, IceDiffuseTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, IceSpecularTexture);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);
        glBindTexture(GL_TEXTURE_2D, 0);

        //Bear
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-3.7, -0.4, -0.2));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.03, 0.03, 0.03));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(120.0f), glm::vec3(0.0, 1.0, 0.0));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, BearDiffuseTexture);
        Bear.Render();

        
        glUseProgram(ColorShader.GetId());
        ColorShader.SetProjection(Projection);
        ColorShader.SetView(View);
       
        
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 1.0f, -2.0f));
        ColorShader.SetModel(ModelMatrix);
        

        //lamp
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-4.1, -1.1, 0.5));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.5f));
        ColorShader.SetModel(ModelMatrix);
        glBindVertexArray(CubeVAO);
        ColorShader.SetUniform3f("uColor", glm::vec3(res, res, 0.0f));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        

        //kocka
        ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.1, pos2, 0.0));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.5, 0.5, 0.5));
        ColorShader.SetModel(ModelMatrix);
        glBindVertexArray(CubeVAO);
        ColorShader.SetUniform3f("uColor", glm::vec3(0.1+pos2, 0.0f, 0.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        ColorShader.SetUniform3f("uColor", glm::vec3(0.0, 0.0f, 0.1+pos2));
        glDrawArrays(GL_TRIANGLES, 6, 6);
        ColorShader.SetUniform3f("uColor", glm::vec3(0.1 + pos2, 0.1 + pos2, 0.0));
        glDrawArrays(GL_TRIANGLES, 12, 6);
        ColorShader.SetUniform3f("uColor", glm::vec3( 0.1 + pos2, 0.0, 0.1 + pos2));
        glDrawArrays(GL_TRIANGLES, 18, 6);
        ColorShader.SetUniform3f("uColor", glm::vec3(0.1 + pos2, 0.1 + pos2, 0.1 + pos2));
        glDrawArrays(GL_TRIANGLES, 24, 6);
        ColorShader.SetUniform3f("uColor", glm::vec3(0.0, 0.1 + pos2, 0.0f ));
        glDrawArrays(GL_TRIANGLES, 30, 6);
        

        // NOTE(Jovan): Draw spotlight
        /*
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 3.5f, -2.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.5f));
        ColorShader.SetModel(ModelMatrix);
        ColorShader.SetUniform3f("uColor", glm::vec3(1.0f, 0.0f, 0.0f));
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        */
        glBindVertexArray(0);
        glUseProgram(0);
        glfwSwapBuffers(Window);

        //Time management
        EndTime = glfwGetTime();
        float WorkTime = EndTime - StartTime;
        if (WorkTime < TargetFrameTime) {
            int DeltaMS = (int)((TargetFrameTime - WorkTime) * 1000.0f);
            std::this_thread::sleep_for(std::chrono::milliseconds(DeltaMS));
            EndTime = glfwGetTime();
        }
        State.mDT = EndTime - StartTime;
    }

    glfwTerminate();
    return 0;
}
