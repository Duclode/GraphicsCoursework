// @global.includes
#include <Shader.h>
#include <camera.h>
#include <model.h>
#include <collision.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <math.h>
#include <string>

//==============================================================================
// @global.callbacks
//==============================================================================
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
unsigned int loadTexture(char const * path);
void setInputMode(GLFWwindow* window, bool enableImgui);; // toggle between camera and UI windows

//==============================================================================
// @global.functions
//==============================================================================
void renderGUI();
void showFrameRateUI();
void showLightsUI();
void syncLightPositionsFromUI();
void initLights(Shader shader, Camera &camera);

//==============================================================================
// @global.state_settings
//==============================================================================

// @window.globals
const unsigned int SCR_WIDTH {800};
const unsigned int SCR_HEIGHT {600};
ImVec4 clear_color {ImVec4(0.1f, 0.1f, 0.1f, 1.0f)};

// @cameras.globals
Camera freeCamera(glm::vec3(-4.2f, 5.8f, 7.5f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -25.0f, FREE);
Camera groundCamera(glm::vec3(-4.2f, 1.75f, 7.5f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -25.0f, FPS);
Camera carouselCamera(glm::vec3(0.50f, 3.3f, -3.25f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -25.0f, STATIONARY);
// Camera pendulumCamera(glm::vec3(-3.8f, 1.9f, 0.25f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -25.0f, STATIONARY);
Camera* currentCamera = &freeCamera;
// float positionss[3] = {-5.00f, -3.41f, -3.25f};
int cameraID {0};
bool justSwitchedCamera = false;

// mouse
bool firstMouse {true};
float lastX {SCR_WIDTH / 2.0f};
float lastY {SCR_HEIGHT / 2.0f};

// timing
float deltaTime {0.0f};  // Time between current frame and last frame
float lastFrame {0.0f};  // Time of last frame

// @lighting.globals
bool toggleFlashLight {true};
float light1Position[3] = {1.2f,  1.0f,  2.0f};
float light1Ambient[3] = {0.1f, 0.1f, 0.1f};
float light1Diffuse[3] = {1.0f, 1.0f, 1.0f};
float light1Specular[3] = {1.0f, 1.0f, 1.0f};
float light2Position[3] = {25.0f,  10.0f,  -4.0f};
float world_light_direction[3] = {-0.2f,  -1.0f,  -0.3f};
float world_light_ambient[3] = {0.05f,  0.05f,  0.05f};
float world_light_diffuse {0.4f};
float world_light_specular[3] = {0.5f,  0.5f,  0.5f};
glm::vec3 pointLightPositions[] = {
    glm::vec3(light1Position[0], light1Position[1], light1Position[2]),
    glm::vec3(light2Position[0], light2Position[1], light2Position[2]),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3( 0.0f,  0.0f, -3.0f)
};

// @imgui.globals
bool show_lights_window {false};
bool imguiMode {false}; // input mode

// @model.globals
float pendulumArmAngle {glm::radians(0.0f)};

int main() {
//==========================================================================
// @app.init
//==========================================================================

    // GLFW: initialize and configure
    // ------------------------------
    // glewExperimental = true;
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialise GLFW\n");
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    //==========================================================================
    // @window.create
    //==========================================================================

    // glfw window creation
    // --------------------
    GLFWwindow* window =
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    /* Create a windowed mode window and its OpenGL context */
    #if defined(GLFW_WAYLAND_APP_ID)
    glfwWindowHintString(GLFW_WAYLAND_APP_ID, "themepark-app");
    #endif
    glfwMakeContextCurrent(window);
    // glfwSwapInterval(1); // Enable vsync
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, keyCallback);

    // tell GLFW to capture mouse
    setInputMode(window, false);

    //==========================================================================
    // @imgui.init
    //==========================================================================

    // Setup Dear ImGui context
    // ------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    // Setup Platform/Renderer backends
    // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    //==========================================================================
    // @gl.init
    //==========================================================================

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Cull triangles which normal is not towards the camera
    // glEnable(GL_CULL_FACE);
    //
    // tell stb_image.h to flip loaded texture's on the y-axis.
    stbi_set_flip_vertically_on_load(true);

    //==========================================================================
    // @shader.load
    // -------------------------------------------------------------------------
    // build and compile shader programs
    //==========================================================================

    Shader modelShader("shaders/glslfiles/modelShader.vert",
                          "shaders/glslfiles/modelShader.frag");

    Shader lightingShader("shaders/glslfiles/colorShader.vert",
                          "shaders/glslfiles/colorShader.frag");

    Shader lightSrcShader("shaders/glslfiles/lightSrcShader.vert",
                          "shaders/glslfiles/lightSrcShader.frag");

    //==========================================================================
    // @model.load
    //==========================================================================

    stbi_set_flip_vertically_on_load(false);
    Model pendulumRideFrameModel("assets/pendulumRide/pendulumFrame.obj");
    Model pendulumRideArmModel("assets/pendulumRide/pendulumArm.obj");
    Model pendulumRideGondolaModel("assets/pendulumRide/pendulumGondola.obj");
    Model groundModel("assets/ground/ground.obj");
    Model carouselTopModel("assets/carousel/top.obj");
    Model carouselHorsesModel("assets/carousel/horses.obj");
    Model carouselBaseModel("assets/carousel/base.obj");
    Model skyBoxModel("assets/skyBox/sky.obj");
    stbi_set_flip_vertically_on_load(true);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    // cube vertices with texture coordinates
    // clang-format off
    float vertices[] = {
        // positions          // texture coords    // normals
        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,   0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,   0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,   0.0f,  0.0f, -1.0f,

        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,   0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,   0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f,  0.0f,  1.0f,

        // Left face
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f,  0.0f,  0.0f,

        // Right face
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   1.0f,  0.0f,  0.0f,

        // Bottom face
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,   0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   0.0f, -1.0f,  0.0f,

        // Top face
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,   0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f,  1.0f,  0.0f
    };
    // clang-format on
    glm::vec3 cubePositions[] = {
        glm::vec3(5.0f, 3.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)};

    //==========================================================================
    // @buffer.cube
    //==========================================================================

    unsigned int VBO{};
    unsigned int cubeVAO{};
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //==========================================================================
    // @buffer.light
    //==========================================================================

    unsigned int lightVAO{};
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // light object uses same VBO as cube obj
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // set to default
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //==========================================================================
    // @texture.load
    //==========================================================================

    unsigned int lightTexture{};

    // Texture 1 -- redstone lamp
    // --------------------------
    glGenTextures(1, &lightTexture);
    // following GL_TEXTURE_2D operations now affect this objecy
    glBindTexture(GL_TEXTURE_2D, lightTexture);
    // set the texture wrapping/filtering options (on the currently bound
    // texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    // GL_NEAREST for pixalated effect, as using minecraft texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // load image, create texture and generate mipmaps
    int width{};
    int height{};
    int nrChannels{};
    unsigned char* data = stbi_load("textures/redstone-lamp.png", &width,
                                    &height, &nrChannels, 0);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // Texture 2 -- container2
    // -----------------------
    unsigned int diffuseMap {loadTexture("textures/container2.png")};

    // Texture 3 -- container2 specular map
    // -----------------------
    unsigned int specularMap {loadTexture("textures/container2_specular.png")};

    // tell opengl for each sampler to which texture unit it belongs to (only
    // has to be done once)
    // -----------------------------------------------------------------------
    // activate the shader before setting uniforms!
    lightSrcShader.use();
    lightSrcShader.setInt("lightTexture", 0);

    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);

    // shader configuration
    // --------------------
    lightingShader.use();

    // material properties
    lightingShader.setFloat("material.shininess", 32.0f);

    modelShader.use();

    // material properties
    modelShader.setFloat("material.shininess", 32.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();

        glm::vec3 previousCameraPosition = currentCamera->Position;

        // input
        // -----
        processInput(window);

        Sphere cameraSphere;
        cameraSphere.center = currentCamera->Position;
        cameraSphere.radius = 0.35f;

        // (Your code calls glfwPollEvents())
        // ...
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // draw gui
        renderGUI();
        showFrameRateUI();

        syncLightPositionsFromUI();

        // render
        // ------
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //======================================================================
        // @camera.matrices
        // ---------------------------------------------------------------------
        // camera/view transformations
        //======================================================================

        if (cameraID == 0) {
            currentCamera = &freeCamera;
        } else if (cameraID == 1) {
            currentCamera = &groundCamera;
        } else if (cameraID == 2) {
            currentCamera = &carouselCamera;
        }
        glm::mat4 projection = glm::perspective(
            glm::radians(currentCamera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f
        );
        glm::mat4 view = currentCamera->GetViewMatrix();

        //======================================================================
        // @lighting.cubes
        // ---------------------------------------------------------------------
        // lighting properties
        //======================================================================

        initLights(lightingShader, *currentCamera);

        //======================================================================
        // @renderer.cubes
        //======================================================================

        // activate shader
        lightingShader.use();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // bind diffuse map Texture on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        glBindVertexArray(cubeVAO);
        for (unsigned int i {0}; i < 1; i++) {
            glm::mat4 model {glm::mat4(1.0f)};
            model = glm::translate(model, cubePositions[i]);
            float angle {20.0f * (float)(i + 1)};  // was i+1
            angle *= (float)glfwGetTime();         // rotate over time
            model = glm::rotate(model, glm::radians(angle),
                                glm::vec3(0.0f, 1.0f, 0.0f));
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        //======================================================================
        // @renderer.lights
        // ---------------------------------------------------------------------
        // draw the lamp object
        //======================================================================
        lightSrcShader.use();
        lightSrcShader.setMat4("projection", projection);
        lightSrcShader.setMat4("view", view);

        // bind Texture on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, lightTexture);

        // render the light object
        glBindVertexArray(lightVAO);
        for (unsigned int i {0}; i < 2; i++) {
            glm::mat4 model {glm::mat4(1.0f)};
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            lightSrcShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);


        //======================================================================
        // @lighting.model
        // ---------------------------------------------------------------------
        // lighting properties
        //======================================================================

        initLights(modelShader, *currentCamera);

        //======================================================================
        // @renderer.models
        // ---------------------------------------------------------------------
        // draw model objects
        //======================================================================

        modelShader.use();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);

        float angle {20.0f};
        angle *= (float)glfwGetTime();         // rotate over time
        // @render.model.carousel
        glm::mat4 baseModel {glm::mat4(1.0f)};
        // translate to center of the scene
        baseModel = glm::translate(baseModel, glm::vec3(25.0f, 0.0f, 0.0f));

        AABB carouselBaseBounds = carouselBaseModel.GetWorldBounds(baseModel);
        if (!justSwitchedCamera && SphereIntersectsAABB(cameraSphere, carouselBaseBounds))
        {
            currentCamera->Position = previousCameraPosition;
        }

        modelShader.setMat4("model", baseModel);
        carouselBaseModel.Draw(modelShader);

        glm::mat4 topModel {baseModel};
        topModel = glm::rotate(topModel, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

        AABB carouselTopBounds = carouselTopModel.GetWorldBounds(baseModel);
        if (!justSwitchedCamera && SphereIntersectsAABB(cameraSphere, carouselTopBounds))
        {
            currentCamera->Position = previousCameraPosition;
        }

        modelShader.setMat4("model", topModel);
        carouselTopModel.Draw(modelShader);

        glm::mat4 horsesModel {topModel};
        horsesModel = glm::translate(horsesModel, glm::vec3(0.0f, (1.0f + sin(glfwGetTime())) * 0.5f, 0.0f));
        modelShader.setMat4("model", horsesModel);
        carouselHorsesModel.Draw(modelShader);
        carouselCamera.UpdatePosition(horsesModel);

        // @render.model.pendulumRide
        glm::mat4 pendulumFrameModel {glm::mat4(1.0f)};
        pendulumFrameModel = glm::translate(pendulumFrameModel, glm::vec3(-3.0f, 5.31f, 0.0f));

        AABB pendulumFrameBounds = pendulumRideFrameModel.GetWorldBounds(pendulumFrameModel);
        if (!justSwitchedCamera && SphereIntersectsAABB(cameraSphere, pendulumFrameBounds))
        {
            currentCamera->Position = previousCameraPosition;
        }

        modelShader.setMat4("model", pendulumFrameModel);
        pendulumRideFrameModel.Draw(modelShader);

        // pendulum arm
        glm::mat4 pendulumArmModel {pendulumFrameModel};
        angle = 40.0f;
        angle *= (float)glfwGetTime();         // rotate over time
        pendulumArmModel = glm::rotate(pendulumArmModel, glm::radians(pendulumArmAngle),
                            glm::vec3(1.0f, 0.0f, 0.0f));
        modelShader.setMat4("model", pendulumArmModel);
        pendulumRideArmModel.Draw(modelShader);
        // pendulumCamera.UpdatePosition(pendulumArmModel); // TODO

        // pendulum gondola
        angle = 40.0f;
        angle *= (float)glfwGetTime();         // rotate over time
        glm::mat4 pendulumGondolaModel {pendulumArmModel};
        pendulumGondolaModel = glm::rotate(pendulumGondolaModel, glm::radians(angle),
                            glm::vec3(0.0f, 1.0f, 0.0f));
        modelShader.setMat4("model", pendulumGondolaModel);
        pendulumRideGondolaModel.Draw(modelShader);
        // pendulumCamera.UpdatePosition(pendulumGondolaModel); // TODO

        // @render.model.ground
        glm::mat4 surfaceModel {glm::mat4(1.0f)};
        surfaceModel = glm::translate(surfaceModel, glm::vec3(0.0f, 0.0f, 0.0f));

        AABB groundBounds = groundModel.GetWorldBounds(surfaceModel);
        if (!justSwitchedCamera && SphereIntersectsAABB(cameraSphere, groundBounds))
        {
            currentCamera->Position = previousCameraPosition;
        }

        modelShader.setMat4("model", surfaceModel);
        groundModel.Draw(modelShader);

        // @render.model.skybox
        glm::mat4 skyModel = glm::mat4(1.0f);
        skyModel = glm::translate(surfaceModel, glm::vec3(0.0f, 0.0f, 0.0f));

        AABB skyBounds = skyBoxModel.GetWorldBounds(surfaceModel);
        if (!justSwitchedCamera && SphereIntersectsAABB(cameraSphere, skyBounds))
        {
            currentCamera->Position = previousCameraPosition;
        }

        modelShader.setMat4("model", skyModel);
        skyBoxModel.Draw(modelShader);

        glBindVertexArray(0); // no need to unbind it every time

        //======================================================================
        // @imgui.render
        // ---------------------------------------------------------------------
        // gui rendering
        //======================================================================
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        justSwitchedCamera = false;
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse
        // moved etc.)
        // -------------------------------------------------------------------
        glfwSwapBuffers(window);
    }

    //==========================================================================
    // @app.shutdown
    // -------------------------------------------------------------------------
    // optional: de-allocate all resources once they've outlived their purpose:
    //==========================================================================
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(lightingShader.ID);
    glDeleteProgram(lightSrcShader.ID);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

//==============================================================================
// @input.keyboard
// -----------------------------------------------------------------------------
// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
//==============================================================================
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (!imguiMode && cameraID == 0) {
        // Camera movement
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            freeCamera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            freeCamera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            freeCamera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            freeCamera.ProcessKeyboard(RIGHT, deltaTime);

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            // TODO
            pendulumArmAngle -= 1.5f;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            // TODO
            pendulumArmAngle += 1.5f;
    } else if (!imguiMode && cameraID == 1) {
        // Camera movement
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            groundCamera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            groundCamera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            groundCamera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            groundCamera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

//==============================================================================
// @input.hotkeys
// -----------------------------------------------------------------------------
// Triggers exactly once per key press
//==============================================================================
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // suppress -Wunused-parameter
    (void)window;
    (void)scancode;
    (void)mods;

    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        toggleFlashLight = !toggleFlashLight;
    }

    // toggles input mode between the camera and the UI windows
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        setInputMode(window, !imguiMode);
    }
}

//==============================================================================
// @window.resize
// -----------------------------------------------------------------------------
// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
//==============================================================================
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;  // suppress -Wunused-parameter
    // make sure the viewport matches the new window dimensions; note that width
    // and height will be significantly larger than specified on retina
    // displays.
    glViewport(0, 0, width, height);
}

//==============================================================================
// @input.mouse
// -----------------------------------------------------------------------------
// process mouse input
//==============================================================================
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    (void)window;  // suppress -Wunused-parameter

    if (imguiMode)
        return;

    float xpos {static_cast<float>(xposIn)};
    float ypos {static_cast<float>(yposIn)};

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset {xpos - lastX};
    float yoffset {lastY - ypos};  // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    freeCamera.ProcessMouseMovement(xoffset, yoffset);
    groundCamera.ProcessMouseMovement(xoffset, yoffset);
    carouselCamera.ProcessMouseMovement(xoffset, yoffset);
}

//==============================================================================
// @input.mode
//==============================================================================
void setInputMode(GLFWwindow* window, bool enableImgui)
{
    imguiMode = enableImgui;

    if (imguiMode) {
        // Give mouse control to ImGui
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        // Capture mouse for FPS camera and make the transition feel smooth
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwFocusWindow(window);
        glfwSetCursorPos(window, static_cast<float>(SCR_WIDTH) / 2.0,
                         static_cast<float>(SCR_HEIGHT) / 2.0);

        lastX = static_cast<float>(SCR_WIDTH) / 2.0f;
        lastY = static_cast<float>(SCR_HEIGHT) / 2.0f;
        firstMouse = true;
    }
}

//==============================================================================
// @input.scroll
// -----------------------------------------------------------------------------
// glfw: whenever the mouse scroll wheel scrolls, this callback is called
//==============================================================================
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;   // suppress -Wunused-parameter
    (void)xoffset;  // suppress -Wunused-parameter
    freeCamera.ProcessMouseScroll(static_cast<float>(yoffset));
    groundCamera.ProcessMouseScroll(static_cast<float>(yoffset));
    carouselCamera.ProcessMouseScroll(static_cast<float>(yoffset));
    // pendulumCamera.ProcessMouseScroll(static_cast<float>(yoffset)); // TODO
}

//==============================================================================
// @texture.load
// -----------------------------------------------------------------------------
// utility function for loading a 2D texture from file
//==============================================================================
unsigned int loadTexture(const char* path)
{
    unsigned int textureID{};
    glGenTextures(1, &textureID);

    int width{};
    int height{};
    int nrComponents{};
    unsigned char *data {stbi_load(path, &width, &height, &nrComponents, 0)};
    if (data)
    {
        GLenum format{};
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

//==============================================================================
// @imgui.main_window
//==============================================================================
void renderGUI() {
    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        ImGui::Begin("Hello, world!", NULL, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text( "Input mode: %s", imguiMode ? "GUI" : "Camera");
        ImGui::Text("Press M to switch modes");
        if (ImGui::Button("Next Camera")){
            cameraID = ((cameraID + 1) % 3); // counter starts from 1
            justSwitchedCamera = true;
        }
        ImGui::SameLine();
        ImGui::Text("Camera: %d", cameraID);
        ImGui::Checkbox("Edit Lights Window", &show_lights_window);
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
        // ImGui::Text("Position, R: %f, G: %f, B: %f", positionss[0], positionss[1], positionss[2]); // TODO
        // ImGui::DragFloat3("Positionss", positionss, 0.01f, -10.0f, 10.0f, "%.3f", ImGuiSliderFlags_None);

        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_lights_window)
    {
        showLightsUI();
    }
}

//==============================================================================
// @imgui.fps
//==============================================================================
void showFrameRateUI() {
    static int location {3};
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;
    const float PAD {10.0f};
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
    window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
    window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
    window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    window_flags |= ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("FPS Gui", NULL, window_flags)) {
        ImGui::Text("FPS\n");
        ImGui::Separator();
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    }
    ImGui::End();
}

//==============================================================================
// @imgui.lighting
//==============================================================================
void showLightsUI() {
    // Pass a pointer to our bool variable (the window will have a closing button that will clear
    // the bool when clicked)
    ImGui::Begin("Edit lights", &show_lights_window, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Edit lights");

    static ImGuiSliderFlags sliderFlags {ImGuiSliderFlags_None};
    static ImGuiTreeNodeFlags nodeFlags {ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed};
    if (ImGui::TreeNodeEx("Directional Light", nodeFlags)) {
        ImGui::Text("World light direction, x: %f, y: %f, z: %f", world_light_direction[0], world_light_direction[1], world_light_direction[2]);
        ImGui::DragFloat3("Light direction", world_light_direction, 0.01f, -1.0f, 0.0f, "%.3f", sliderFlags);
        ImGui::Text("World light diffuse intensity: %f", world_light_diffuse);
        ImGui::DragFloat("Diffuse", &world_light_diffuse, 0.01f, 0.0f, 1.0f, "%.2f", sliderFlags);
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Point Light 1", nodeFlags)) {
        ImGui::Text("Light 1 pos, x: %.2f, y: %.2f, z: %.2f", light1Position[0], light1Position[1], light1Position[2]);
        ImGui::DragFloat3("Position", light1Position, 0.01f, -24.0f, 24.0f, "%.3f", sliderFlags); ImGui::SameLine();
        if (ImGui::Button("reset pos")) { light1Position[0] = {1.2f}; light1Position[1] = {1.0f}; light1Position[2] = {2.0f};}
        ImGui::Text("Light 1 Ambient, R: %f, G: %f, B: %f", light1Ambient[0], light1Ambient[1], light1Ambient[2]);
        ImGui::DragFloat3("Ambient", light1Ambient, 0.01f, -15.0f, 15.0f, "%.3f", sliderFlags); ImGui::SameLine();
        if (ImGui::Button("reset amb")) { light1Ambient[0] = {0.1f}; light1Ambient[1] = {0.1f}; light1Ambient[2] = {0.1f};}
        ImGui::Text("Light 1 Diffuse, R: %f, G: %f, B: %f", light1Diffuse[0], light1Diffuse[1], light1Diffuse[2]);
        ImGui::DragFloat3("Diffuse", light1Diffuse, 0.01f, -15.0f, 15.0f, "%.3f", sliderFlags); ImGui::SameLine();
        if (ImGui::Button("reset diff")) { light1Diffuse[0] = {1.0f}; light1Diffuse[1] = {1.0f}; light1Diffuse[2] = {1.0f};}
        ImGui::Text("Light 1 Specular, R: %f, G: %f, B: %f", light1Specular[0], light1Specular[1], light1Specular[2]);
        ImGui::DragFloat3("Specular", light1Specular, 0.01f, -15.0f, 15.0f, "%.3f", sliderFlags); ImGui::SameLine();
        if (ImGui::Button("reset spec")) { light1Specular[0] = {1.0f}; light1Specular[1] = {1.0f}; light1Specular[2] = {1.0f};}
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Point Light 2", nodeFlags)) {
        ImGui::Text("Light 2 pos, x: %f, y: %f, z: %f", light2Position[0], light2Position[1], light2Position[2]);
        ImGui::DragFloat3("Position", light2Position, 0.01f, -5.0f, 5.0f, "%.3f", sliderFlags);
        ImGui::TreePop();
    }

    if (ImGui::Button("Close Me"))
        show_lights_window = false;

    syncLightPositionsFromUI();
    ImGui::End();
}

void syncLightPositionsFromUI() {
    pointLightPositions[0] = glm::vec3(
        light1Position[0], light1Position[1], light1Position[2]);
    pointLightPositions[1] = glm::vec3(
        light2Position[0], light2Position[1], light2Position[2]);
}

//======================================================================
// @lighting.init
// ---------------------------------------------------------------------
// lighting properties
//======================================================================
void initLights(Shader shader, Camera &camera) {
    // activate shader
    shader.use();
    shader.setVec3("viewPos", camera.Position);

    // material shininess
    shader.setFloat("material.shininess", 32.0f);
    // directional Light
    shader.setVec3("dirLight.direction", world_light_direction[0], world_light_direction[1], world_light_direction[2]); // global light dir
    shader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    shader.setVec3("dirLight.diffuse", world_light_diffuse, world_light_diffuse, world_light_diffuse);  // brightness
    shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    // point light 1
    shader.setVec3("pointLights[0].position", pointLightPositions[0]); // light pos
    shader.setVec3("pointLights[0].ambient", glm::vec3(light1Ambient[0], light1Ambient[1], light1Ambient[2]));
    shader.setVec3("pointLights[0].diffuse", glm::vec3(light1Diffuse[0], light1Diffuse[1], light1Diffuse[2])); // brightness
    shader.setVec3("pointLights[0].specular", glm::vec3(light1Specular[0], light1Specular[1], light1Specular[2]));
    // attenuation
    shader.setFloat("pointLights[0].constant",  1.0f); // 50 metres
    shader.setFloat("pointLights[0].linear",    0.09f);
    shader.setFloat("pointLights[0].quadratic", 0.032f);
    // point light 2
    shader.setVec3("pointLights[1].position", pointLightPositions[1]);
    shader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
    shader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f); // brightness
    shader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("pointLights[1].constant", 1.0f);
    shader.setFloat("pointLights[1].linear", 0.09f);
    shader.setFloat("pointLights[1].quadratic", 0.032f);
    // flashlight
    shader.setVec3("flashLight.position",  camera.Position);
    shader.setVec3("flashLight.direction", camera.Front);
    shader.setFloat("flashLight.cutOff",   glm::cos(glm::radians(12.5f)));
    shader.setFloat("flashLight.outerCutOff",   glm::cos(glm::radians(17.5f)));
    shader.setVec3("flashLight.ambient", 0.1f, 0.1f, 0.1f);
    shader.setVec3("flashLight.diffuse", 1.0f, 1.0f, 1.0f); // brightness
    shader.setVec3("flashLight.specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("flashLight.constant",  1.0f); // 50 metres
    shader.setFloat("flashLight.linear",    0.09f);
    shader.setFloat("flashLight.quadratic", 0.032f);
    shader.setBool("flashLight.toggle", toggleFlashLight);
}
