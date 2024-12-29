#include <iostream>
#include <cmath>
#include <ctime>

// Include GLAD
#include <glad/glad.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Vertex shader for rendering a full-screen quad.
const char* vertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

out vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

// Fragment shader implementing the 2D noise.
const char* fragmentShaderSource = R"(
// fragmentShader.glsl
#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform float u_time;
uniform vec2  u_resolution;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    // Normalize UV if we want gl_FragCoord style behavior:
    // If you're using a full-screen quad with vUV in [0..1], no need to normalize again.
    // Otherwise, if you want to match the earlier example:
    // vec2 uv = vUV; // if your quad's UV is already (0..1)
    vec2 uv = vUV;

    // Optional animation over time
    // uv += floor(u_time * 60.0);

    float n = hash(uv * u_resolution);

    FragColor = vec4(vec3(n), 1.0);
}

)";

// Helper function to check for shader compile errors
void checkCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " 
                      << type << "\n" << infoLog << "\n";
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " 
                      << type << "\n" << infoLog << "\n";
        }
    }
}

// Create a full-screen quad
//   Position      UV
//   (-1, -1) -> (0, 0)
//   ( 1, -1) -> (1, 0)
//   ( 1,  1) -> (1, 1)
//   (-1,  1) -> (0, 1)

float vertices[] = {
    //   X     Y     U     V
    -1.f, -1.f,    0.f,  0.f, 
     1.f, -1.f,    1.f,  0.f,
     1.f,  1.f,    1.f,  1.f,

    -1.f, -1.f,    0.f,  0.f,
     1.f,  1.f,    1.f,  1.f,
    -1.f,  1.f,    0.f,  1.f
};

int main()
{
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!\n";
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // We’ll use Core Profile for modern OpenGL
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "2D Noise Shader", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window!\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!\n";
        return -1;
    }

    // Viewport
    glViewport(0, 0, 800, 600);

    // Build and compile shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");

    // Link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkCompileErrors(shaderProgram, "PROGRAM");

    // Delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Generate VAO/VBO for the quad
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // UV attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Use our shader program
    glUseProgram(shaderProgram);

    // Retrieve uniform locations
    GLint timeLoc        = glGetUniformLocation(shaderProgram, "u_time");
    GLint resolutionLoc  = glGetUniformLocation(shaderProgram, "u_resolution");

    // Set the resolution uniform once (if your window size changes, you’d update this)
    glUniform2f(resolutionLoc, 800.0f, 600.0f);

    // Game/Render loop
    float startTime = (float)glfwGetTime();  
    while (!glfwWindowShouldClose(window))
    {
        // Calculate elapsed time
        float currentTime = (float)glfwGetTime();
        float elapsed = currentTime - startTime;

        // Set background color and clear
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Use our shader program
        glUseProgram(shaderProgram);

        // Update the time uniform
        glUniform1f(timeLoc, elapsed);

        // Render the full-screen quad
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // Terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

