#include "utils.h"
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLShader::GLShader(std::string glsl_file_path, bool load_geometry)
{
    mProgram = glCreateProgram();
    GLSL = glsl_file_path;
    AttachGLSL(GLSL + ".vs", GL_VERTEX_SHADER);
    AttachGLSL(GLSL + ".fs", GL_FRAGMENT_SHADER);
    if (load_geometry)
        AttachGLSL(GLSL + ".gs", GL_GEOMETRY_SHADER);
    glUseProgram(mProgram);
}

void GLShader::AttachGLSL(std::string glsl_file_path, GLenum type)
{
    std::string glslCode;
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        auto glslPath = glsl_file_path;
        shaderFile.open(glslPath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        glslCode = shaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ  PATH:" << glsl_file_path << std::endl;
    }
    const char *shaderCode = glslCode.c_str();
    GLuint tmpShader = glCreateShader(type);
    glShaderSource(tmpShader, 1, &shaderCode, NULL);
    glCompileShader(tmpShader);
    int success;
    char infoLog[512];
    glGetShaderiv(tmpShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(tmpShader, 512, NULL, infoLog);
        if(type == GL_VERTEX_SHADER)
        {
            
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
        }
        else if(type == GL_FRAGMENT_SHADER)
        {
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
        }
        else if(type == GL_GEOMETRY_SHADER)
        {
            std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
        }
    }
    glAttachShader(mProgram, tmpShader);
    glLinkProgram(mProgram);
    glDeleteShader(tmpShader);
}

GLuint GLShader::GetShaderProgram()
{
    return mProgram;
}

void GLShader::SetUniform(const std::string &name, int value)
{
    glUseProgram(mProgram);
    glUniform1i(glGetUniformLocation(mProgram, name.c_str()), value);
}

void GLShader::SetUniform(const std::string &name, float value)
{
    glUseProgram(mProgram);
    glUniform1f(glGetUniformLocation(mProgram, name.c_str()), value);
}

void GLShader::SetUniform(const std::string &name, bool value)
{
    glUseProgram(mProgram);
    glUniform1i(glGetUniformLocation(mProgram, name.c_str()), (int)value);
}

void GLShader::SetUniform(const std::string &name, glm::mat4 mat4)
{
    glUseProgram(mProgram);
    glUniformMatrix4fv(glGetUniformLocation(mProgram, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat4));
}

void GLShader::SetUniform(const std::string &name, glm::mat3 mat3)
{
    glUseProgram(mProgram);
    glUniformMatrix3fv(glGetUniformLocation(mProgram, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat3));
}

void GLShader::SetUniform(const std::string &name, glm::vec3 vec3)
{
    glUseProgram(mProgram);
    glUniform3fv(glGetUniformLocation(mProgram, name.c_str()), 1, glm::value_ptr(vec3));
}

GLShader::~GLShader()
{
    glDeleteProgram(mProgram);
}

GLuint LoadTexture(const char *file_path, GLint mode, bool gamma)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(file_path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << file_path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

int GlfwGladInitialization(GLFWwindow **window, int width, int height, const char *title)
{
    // 初始化
    glfwInit();
    // 设置主版本号 3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // 设置副版本号 3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // GLFW_OPENGL_CORE_PROFILE 对应核心模式
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    *window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // 设置当前上下文
    glfwMakeContextCurrent(*window);
    // 初始化GLAD, GLAD回将所有设备的地址绑定到对应的指针上以供我们使用
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    return 0;
}

void FramebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void RenderSphere()
{
    static unsigned int sphereVAO = 0;
    static unsigned int indexCount;
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;
        std::vector<glm::vec3> tangents, bitangents;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float theta = xSegment * 2.0f * PI;
                float phi = ySegment * PI;

                glm::vec3 n(
                    std::cos(theta) * std::sin(phi),
                    std::cos(phi),
                    std::sin(theta) * std::sin(phi)
                );
                glm::vec3 t(
                    -std::sin(theta),
                    0.0f,
                    std::cos(theta)
                );
                glm::vec3 b(
                    std::cos(theta) * std::cos(phi),
                    -std::sin(phi),
                    std::sin(theta) * std::cos(phi)
                );
                tangents.push_back(glm::normalize(t));
                bitangents.push_back(glm::normalize(b));

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }


        indexCount = static_cast<unsigned int>(indices.size());

        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
            if (tangents.size() > 0)
            {
                data.push_back(tangents[i].x);
                data.push_back(tangents[i].y);
                data.push_back(tangents[i].z);
            }
            if (bitangents.size() > 0)
            {
                data.push_back(bitangents[i].x);
                data.push_back(bitangents[i].y);
                data.push_back(bitangents[i].z);
            }
            
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        unsigned int stride = (3 + 2 + 3 + 3 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void *)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void *)(11 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}


void RenderQuad()
{
    static unsigned int quadVAO = 0;
    static unsigned int quadVBO;
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            -1.0f,
            -1.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            -1.0f,
            0.0f,
            1.0f,
            0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}