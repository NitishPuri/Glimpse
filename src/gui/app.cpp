#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <iostream>
#include <vector>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

// Window dimensions
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Vertex Shader
const char *vertexShaderSrc = R"(   
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec3 aColor;
    out vec3 vColor;
    void main() {
        vColor = aColor;
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
)";

// Fragment Shader
const char *fragmentShaderSrc = R"(
    #version 330 core
    in vec3 vColor;
    out vec4 FragColor;
    void main() {
        FragColor = vec4(vColor, 1.0);
    }
)";

struct GlResources
{
    // Quad
    GLuint VBO, VAO, EBO;
    GLuint shaderProgram;

    // Texture
    GLuint framebufferTexture;
    int renderWidth = WINDOW_WIDTH / 2;
    int renderHeight = WINDOW_HEIGHT / 2;

    // spice
    int duration = 0;

} Resources;

struct ImguiParams
{
    float backgroundColor[3] = {0.1f, 0.1f, 0.1f};
} Params;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Create a simple shader program
GLuint createShaderProgram()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

std::vector<uint8_t> raytracer(int width, int height)
{
    std::vector<uint8_t> image(width * height * 3); // RGB image
    // Your ray tracing logic here
    Resources.duration++;
    auto s = sin(Resources.duration * 0.1);
    auto c = cos(Resources.duration * 0.1);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // Compute pixel color
            int index = (y * width + x) * 3;
            image[index] = int(c * 255);     // R
            image[index + 1] = int(s * 255); // G
            image[index + 2] = 0;            // B
        }
    }
    return image;
}

void setupQuad()
{
    // Quad Data
    float vertices[] = {
        -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, // Bottom-left (red)
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // Bottom-right (green)
        0.5f, 0.5f, 0.0f, 0.0f, 1.0f,   // Top-right (blue)
        -0.5f, 0.5f, 1.0f, 1.0f, 0.0f   // Top-left (yellow)
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0};

    glGenVertexArrays(1, &Resources.VAO);
    glGenBuffers(1, &Resources.VBO);
    glGenBuffers(1, &Resources.EBO);

    glBindVertexArray(Resources.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, Resources.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Resources.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    Resources.shaderProgram = createShaderProgram();

    // Generate a texture for the framebuffer
    glGenTextures(1, &Resources.framebufferTexture);
    glBindTexture(GL_TEXTURE_2D, Resources.framebufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Resources.renderWidth, Resources.renderHeight,
                 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

// Function to update the framebuffer texture with the rendered image
void updateFramebuffer(const std::vector<uint8_t> &imageData)
{
    glBindTexture(GL_TEXTURE_2D, Resources.framebufferTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Resources.renderWidth, Resources.renderHeight,
                    GL_RGB, GL_UNSIGNED_BYTE, imageData.data());
}

void renderUI()
{
    ImGui::Begin("Control Panel");
    ImGui::ColorEdit3("Quad Color", Params.backgroundColor);

    // if (ImGui::Button("Render"))
    {
        // Render the scene
        std::vector<uint8_t> imageData = raytracer(Resources.renderWidth, Resources.renderHeight);
        updateFramebuffer(imageData);
    }

    ImGui::End();
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Glimpse", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    // glfwSwapInterval(1); // Enable vsync
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // ImGuiIO &io = ImGui::GetIO();
    // (void)io;
    // ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    setupQuad();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui UI
        renderUI();

        // Clear screen
        glClearColor(Params.backgroundColor[0], Params.backgroundColor[1], Params.backgroundColor[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::Begin("Render Output");
        ImGui::Image(Resources.framebufferTexture,
                     ImVec2(float(Resources.renderWidth), float(Resources.renderHeight)));
        ImGui::End();

        // Render Quad
        // glUseProgram(Resources.shaderProgram);
        // glBindVertexArray(Resources.VAO);
        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
