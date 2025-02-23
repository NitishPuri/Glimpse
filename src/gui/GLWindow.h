// // clang-format off
// #include "glad/glad.h"
// #include "GLFW/glfw3.h"
// // clang-format on

// // Window dimensions
// const int WINDOW_WIDTH = 1800;
// const int WINDOW_HEIGHT = 1600;

// // Vertex Shader
// const char *vertexShaderSrc = R"(
//   #version 330 core
//   layout (location = 0) in vec2 aPos;
//   layout (location = 1) in vec3 aColor;
//   out vec3 vColor;
//   void main() {
//       vColor = aColor;
//       gl_Position = vec4(aPos, 0.0, 1.0);
//   }
// )";

// // Fragment Shader
// const char *fragmentShaderSrc = R"(
//   #version 330 core
//   in vec3 vColor;
//   out vec4 FragColor;
//   void main() {
//       FragColor = vec4(vColor, 1.0);
//   }
// )";

// struct GlResources {
//   // Quad
//   GLuint VBO, VAO, EBO;
//   GLuint shaderProgram;

//   // Texture
//   GLuint framebufferTexture;
//   int renderWidth = WINDOW_WIDTH / 2;
//   int renderHeight = WINDOW_HEIGHT / 2;

// } Resources;

// // Create a simple shader program
// GLuint createShaderProgram() {
//   GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
//   glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
//   glCompileShader(vertexShader);

//   GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//   glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
//   glCompileShader(fragmentShader);

//   GLuint shaderProgram = glCreateProgram();
//   glAttachShader(shaderProgram, vertexShader);
//   glAttachShader(shaderProgram, fragmentShader);
//   glLinkProgram(shaderProgram);

//   glDeleteShader(vertexShader);
//   glDeleteShader(fragmentShader);

//   return shaderProgram;
// }
