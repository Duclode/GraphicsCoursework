#include <Shader.h>
#include <camera.h>
#include <model.h>

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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
unsigned int loadTexture(char const * path);

// settings
const unsigned int SCR_WIDTH {800};
const unsigned int SCR_HEIGHT {600};

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f)); // 0.5f, 0.8f, 4.0f

// mouse
bool firstMouse {true};
float lastX {SCR_WIDTH / 2.0f};
float lastY {SCR_HEIGHT / 2.0f};

// timing
float deltaTime {0.0f};  // Time between current frame and last frame
float lastFrame {0.0f};  // Time of last frame

// texture
// stores how much we're seeing of either texture
float mixValue {0.2f};

// lighting
bool toggleFlashLight {true};

int main() {
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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


    // build and compile shader programs
    // ------------------------------------
    Shader modelShader("shaders/glslfiles/modelShader.vert",
                          "shaders/glslfiles/modelShader.frag");

    Shader lightingShader("shaders/glslfiles/colorShader.vert",
                          "shaders/glslfiles/colorShader.frag");

    Shader lightSrcShader("shaders/glslfiles/lightSrcShader.vert",
                          "shaders/glslfiles/lightSrcShader.frag");

    // load models
    // -----------
    Model backpackModel("assets/backpack/backpack.obj");

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
        glm::vec3(2.0f, -0.5f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)};

    glm::vec3 pointLightPositions[] = {
        glm::vec3( 1.2f,  1.0f,  2.0f),
        glm::vec3( 2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3( 0.0f,  0.0f, -3.0f)
    };

    unsigned int VBO, cubeVAO;
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

    // second, configure the light's VAO (VBO stays the same; the vertices are
    // the same for the light object which is also a 3D cube)
    unsigned int lightVAO{};
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // we only need to bind to the VBO, the container's VBO's data already
    // contains the data.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Textures 1 -- restone lamp
    // --------------------------
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

    // lighting properties
    // Directional Light
    lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f); // global light dir
    lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);  // brightness
    lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    // point light 1
    lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]); // light pos
    lightingShader.setVec3("pointLights[0].ambient", 0.1f, 0.1f, 0.1f);
    lightingShader.setVec3("pointLights[0].diffuse", 1.0f, 1.0f, 1.0f); // brightness
    lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("pointLights[0].constant",  1.0f); // 50 metres
    lightingShader.setFloat("pointLights[0].linear",    0.09f);
    lightingShader.setFloat("pointLights[0].quadratic", 0.032f);
    // point light 2
    lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
    lightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f); // brightness
    lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("pointLights[1].constant", 1.0f);
    lightingShader.setFloat("pointLights[1].linear", 0.09f);
    lightingShader.setFloat("pointLights[1].quadratic", 0.032f);

    modelShader.use();

    // material properties
    modelShader.setFloat("material.shininess", 32.0f);

    // lighting properties
    // Directional Light
    modelShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f); // global light dir
    modelShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    modelShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);  // brightness
    modelShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    // point light 1
    modelShader.setVec3("pointLights[0].position", pointLightPositions[0]); // light pos
    modelShader.setVec3("pointLights[0].ambient", 0.1f, 0.1f, 0.1f);
    modelShader.setVec3("pointLights[0].diffuse", 1.0f, 1.0f, 1.0f); // brightness
    modelShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    modelShader.setFloat("pointLights[0].constant",  1.0f); // 50 metres
    modelShader.setFloat("pointLights[0].linear",    0.09f);
    modelShader.setFloat("pointLights[0].quadratic", 0.032f);
    // point light 2
    modelShader.setVec3("pointLights[1].position", pointLightPositions[1]);
    modelShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
    modelShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f); // brightness
    modelShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
    modelShader.setFloat("pointLights[1].constant", 1.0f);
    modelShader.setFloat("pointLights[1].linear", 0.09f);
    modelShader.setFloat("pointLights[1].quadratic", 0.032f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);
        glfwSetKeyCallback(window, keyCallback);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // activate shader
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);

        // flashlight
        lightingShader.setVec3("flashLight.position",  camera.Position);
        lightingShader.setVec3("flashLight.direction", camera.Front);
        lightingShader.setFloat("flashLight.cutOff",   glm::cos(glm::radians(12.5f)));
        lightingShader.setFloat("flashLight.outerCutOff",   glm::cos(glm::radians(17.5f)));
        lightingShader.setVec3("flashLight.ambient", 0.1f, 0.1f, 0.1f);
        lightingShader.setVec3("flashLight.diffuse", 1.0f, 1.0f, 1.0f); // brightness
        lightingShader.setVec3("flashLight.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("flashLight.constant",  1.0f); // 50 metres
        lightingShader.setFloat("flashLight.linear",    0.09f);
        lightingShader.setFloat("flashLight.quadratic", 0.032f);
        lightingShader.setBool("flashLight.toggle", toggleFlashLight);

        // camera/view transformations
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // update light color over time
        // ----------------------------------------------------------
        /**
        glm::vec3 lightColor;
        lightColor.x = (float)sin(glfwGetTime() * 1.0f) + 1.0f;
        lightColor.y = (float)sin(glfwGetTime() * 1.0f) + 1.0f;
        lightColor.z = (float)sin(glfwGetTime() * 1.0f) + 1.0f;

        glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);

        lightingShader.setVec3("light.ambient", ambientColor);
        lightingShader.setVec3("light.diffuse", diffuseColor);
        **/
        //\\----------------------------------------------------------

        // bind diffuse map Texture on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        // render boxes
        glBindVertexArray(cubeVAO);
        for (unsigned int i {0}; i < 1; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * (float)(i + 1);  // was i+1
            angle *= (float)glfwGetTime();         // rotate over time
            model = glm::rotate(model, glm::radians(angle),
                                glm::vec3(0.0f, 1.0f, 0.0f));
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        // draw the lamp object
        lightSrcShader.use();
        lightSrcShader.setMat4("projection", projection);
        lightSrcShader.setMat4("view", view);

        // bind Texture on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, lightTexture);

        // render the light object
        glBindVertexArray(lightVAO);
        for (unsigned int i {0}; i < 2; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            lightSrcShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        // draw model objects
        modelShader.use();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);

        modelShader.setVec3("viewPos", camera.Position);

        // flashlight
        modelShader.setVec3("flashLight.position",  camera.Position);
        modelShader.setVec3("flashLight.direction", camera.Front);
        modelShader.setFloat("flashLight.cutOff",   glm::cos(glm::radians(12.5f)));
        modelShader.setFloat("flashLight.outerCutOff",   glm::cos(glm::radians(17.5f)));
        modelShader.setVec3("flashLight.ambient", 0.1f, 0.1f, 0.1f);
        modelShader.setVec3("flashLight.diffuse", 1.0f, 1.0f, 1.0f); // brightness
        modelShader.setVec3("flashLight.specular", 1.0f, 1.0f, 1.0f);
        modelShader.setFloat("flashLight.constant",  1.0f); // 50 metres
        modelShader.setFloat("flashLight.linear",    0.09f);
        modelShader.setFloat("flashLight.quadratic", 0.032f);
        modelShader.setBool("flashLight.toggle", toggleFlashLight);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        // translate to center of the scene
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        // scale down model to fit scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        modelShader.setMat4("model", model);
        backpackModel.Draw(modelShader);

        glBindVertexArray(0); // no need to unbind it every time

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse
        // moved etc.)
        // -------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(lightingShader.ID);
    glDeleteProgram(lightSrcShader.ID);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        mixValue += 0.01f;  // change this value accordingly (might be too slow
                            // or too fast based on system hardware)
        if (mixValue >= 1.0f) mixValue = 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        mixValue -= 0.01f;  // change this value accordingly (might be too slow
                            // or too fast based on system hardware)
        if (mixValue <= 0.0f) mixValue = 0.0f;
    }

    // Camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Triggers exactly once per key press
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // suppress -Wunused-parameter
    (void)window;
    (void)scancode;
    (void)mods;
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        toggleFlashLight = !toggleFlashLight;
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ----------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;  // suppress -Wunused-parameter
    // make sure the viewport matches the new window dimensions; note that width
    // and height will be significantly larger than specified on retina
    // displays.
    glViewport(0, 0, width, height);
}

// process mouse input
// -------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    (void)window;  // suppress -Wunused-parameter
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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;   // suppress -Wunused-parameter
    (void)xoffset;  // suppress -Wunused-parameter
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
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
