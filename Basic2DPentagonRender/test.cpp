#include <alloca.h>
#include <csignal>
#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ostream>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

#define ASSERT(x) if (!(x)) raise(SIGTRAP); 
#define GLCall(x) GLClearError();\
  x;\
  ASSERT(GLLogCall(#x, __FILE__, __LINE__))

static void GLClearError(){
  while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, int line){
  while (GLenum error = glGetError()) {
    std::cout<< "OpenGL Error :" << error << " "<< function << " " << file << ":" << line << std::endl;
    return  false;
  }
  return  true;
}


struct ShaderProgramSource{
  std::string VertexSource;
  std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath){
  std::ifstream stream(filepath);
  
  enum class ShaderType{
     NONE = -1, VERTEX = 0, FRAGMENT = 1,
  };


  std::string line;
  std::stringstream ss[2];
  ShaderType type = ShaderType::NONE;

  while(getline(stream, line)){
    if(line.find("#shader") != std::string::npos){
      if(line.find("vertex") != std::string::npos){
        type = ShaderType::VERTEX;
      }
      else if(line.find("fragment") != std::string::npos){
        type = ShaderType::FRAGMENT;
      }
    }
    else{
      ss[(int)type] << line << '\n';
    }
  }

  return {ss[0].str(), ss[1].str()}; 
}

static unsigned int CompileShader(unsigned int type, const std::string& source){
  unsigned int id = glCreateShader(type);
  const char* src = source.c_str();
  glShaderSource(id,1, &src, nullptr);
  glCompileShader(id);


  int result;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if(result == GL_FALSE){
    int length;
    glGetShaderiv(id,GL_INFO_LOG_LENGTH, &length);
    char* message =(char*)alloca(length * sizeof(char));
    glGetShaderInfoLog(id,length,&length,message);
    std::cout << message << std::endl;
    glDeleteShader(id);
    return 0;
  }
  return  id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader){
  
  unsigned int program = glCreateProgram();
  unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
  unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

  glAttachShader(program,vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glValidateProgram(program);

  glDeleteShader(vs);
  glDeleteShader(fs);

  return  program;
}
int main () {
  //glfw set up
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window = glfwCreateWindow(1920, 1080, "LearnOpenGL", NULL, NULL);
  
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window); 
  
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
    
  std::cout << glGetString(GL_VERSION) << std::endl;
  
  //vertex buffer
  
  /*float positions[] = {*/
  /*  -0.5f, -0.5f,*/
  /*   0.5f, -0.5f,*/
  /*   0.5f,  0.5f,*/
  /*  -0.5f,  0.5f,*/
  /*};*/
  /**/
  /*unsigned int indices[] = {*/
  /*    0,1,2,*/
  /*    2,3,0*/
  /*};*/

  float positions[] = {
    0.0f, 1.0f,                   // Vertex 0
    0.951f, 0.309f,               // Vertex 1
    0.588f, -0.809f,              // Vertex 2
    -0.588f, -0.809f,             // Vertex 3
    -0.951f, 0.309f   
  };

  

  unsigned int indices[] = {
    0, 1, 2,
    0, 2, 3,
    0, 3, 4
  };

  unsigned int VAO;
  GLCall(glGenVertexArrays(1, &VAO));
  GLCall(glBindVertexArray(VAO));

  unsigned int buffer;
  GLCall(glGenBuffers(1, &buffer));
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
  GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions , GL_STATIC_DRAW));

  unsigned int ibo;
  GLCall(glGenBuffers(1, &ibo));
  GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
  GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),indices,GL_STATIC_DRAW));

  GLCall(glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(float) * 2, (void*)0));
  GLCall(glEnableVertexAttribArray(0));

  // Shader 
  ShaderProgramSource source= ParseShader("./res/shaders/Basic.shader");

  unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    
  GLCall(glUseProgram(shader));

  while (!glfwWindowShouldClose(window)) {
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
    
    GLCall(glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, nullptr));
  
    GLCall(glfwSwapBuffers(window));

    GLCall(glfwPollEvents());
  }
  
  GLCall(glDeleteProgram(shader));
  GLCall(glfwTerminate());
  
  return 0;
}
