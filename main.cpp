#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <sstream>
#include <fstream>

// stb
#include "./include/stb_image.h"
#include "./include/glm/glm.hpp"
#include "./include/glm/gtc/matrix_transform.hpp"

// imgui
#include "./include/imGui/imgui.h"
#include "./include/imGui/backends/imgui_impl_glfw.h"
#include "./include/imGui/backends/imgui_impl_opengl3.h"

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

std::string parserShader(const std::string& filepath)
{
    std::ifstream stream;
    stream.open(filepath);
    if (!stream.is_open())
    {
        std::cout << "Open file failed: " << filepath << std::endl;
    }

    std::string line;
    std::stringstream ss;

    while (std::getline(stream, line))
    {
        ss << line << "\n";
    }

    return ss.str();
}

GLuint compileShader(GLuint type, const std::string& shader)
{
    GLuint id = glCreateShader(type);
    const char* src = shader.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    // 错误处理
    GLint result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        GLint length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* msg = (char*)malloc(sizeof(char) * length);
        glGetShaderInfoLog(id, length, &length, msg);
        
        std::cout << "Failed to compile shader" << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << std::endl;
        std::cout << msg << std::endl;
        
        glDeleteShader(id);
        return 0;
    }
    return id;
}

GLuint createShader(const std::string& vertShader, const std::string& fragShader)
{
    GLuint program = glCreateProgram();
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertShader);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int main()
{
    GLFWwindow* window;

    // 初始化glfw库
    if (glfwInit() == GLFW_FALSE)
    {
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // for mac
#endif
    // 创建窗口
    window = glfwCreateWindow(1024, 568, "Hello OpenGL!", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 允许键盘控制
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // 允许游戏手柄控制

    // 设置渲染器后端
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    float positions[] = {
        -0.5f, -0.5f, 0.0f, 0.0f,
         0.5f, -0.5f, 1.0f, 0.0f,
         0.5f,  0.5f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 1.0f
    };    

    unsigned int indices[] =
    {
        0, 1, 2,
        2, 3, 0
    };

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(float), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(float) * 4,
        reinterpret_cast<void*>(0)
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(float) * 4,
        reinterpret_cast<void*>(sizeof(float) * 2)
    );

    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * 2 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    // 模型矩阵
    float degree = 0.0f;
    glm::vec3 rotate_axis = {1.0f, 0.0f, 0.0f};
    glm::vec3 translation(0.0f, 0.0f, 0.0f);
    glm::mat4 model = glm::mat4(1.0);
    model = glm::rotate(model, glm::radians(float(degree)), rotate_axis);
    model = glm::translate(model, translation);

    // 观察矩阵
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

    // 正交投影矩阵
    float fov = 45.0;
    glm::mat4 proj(1.0);
    proj = glm::perspective(glm::radians(float(fov)), (float)1024.0 / (float)568.0, 0.1f, 1000.0f);

    // std::string vertShader = parserShader("../test/Shaders/Basic.vert");
    // std::string fragShader = parserShader("../test/Shaders/Basic.frag");
    // GLuint program = createShader(vertShader, fragShader);

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(shaderProgram);

    GLint loc = glGetUniformLocation(shaderProgram, "uWorldTransPos");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &model[0][0]);
    loc = glGetUniformLocation(shaderProgram, "uViewProj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &((proj * view)[0][0]));

    /**********  渲染2D纹理  ************/
    // 加载图片
    int width, height, channel;
    // 翻转图像
    stbi_set_flip_vertically_on_load(true);
    unsigned char* image = stbi_load("./assets/donkey.png", &width, &height, &channel, STBI_default);
    if (!image)
    {
        std::cout << "Load image failed!" << std::endl;
        return -1;
    }

    int format = GL_RGB;
    if (channel == 4)
    {
        format = GL_RGBA;
    }

    // 创建纹理缓存区
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // 双线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
    
    glActiveTexture(GL_TEXTURE0);
    GLint texLoc = glGetUniformLocation(shaderProgram, "uTexture");
    glUniform1i(texLoc, 0);

    while (!glfwWindowShouldClose(window))
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        model = glm::rotate(glm::mat4(1.0), glm::radians(degree), rotate_axis);
        model = glm::translate(model, translation);
        proj = glm::perspective(glm::radians(fov), (float)1024.0 / (float)568.0, 0.1f, 1000.0f);
        glm::mat4 mvp = proj * view * model;
        
        GLint loc = glGetUniformLocation(shaderProgram, "uWorldTransPos");
        glUniformMatrix4fv(loc, 1, GL_FALSE, &model[0][0]);
        loc = glGetUniformLocation(shaderProgram, "uViewProj");
        glUniformMatrix4fv(loc, 1, GL_FALSE, &((proj * view)[0][0]));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        ImGui::Text("Avg fps: %.3f", ImGui::GetIO().Framerate);
        ImGui::SliderFloat("Degree", &degree, -180.0f, 180.0f);
        ImGui::SliderFloat3("Roation_Axis.xyz", &rotate_axis.x, -1.0, 1.0);
        ImGui::SliderFloat3("Translation.xyz", &translation.x, -1.0, 1.0);
        ImGui::SliderFloat("Fov", &fov, 1.0f, 360.0f);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}