#pragma once
#include "glad/glad.h"
#include "KHR/khrplatform.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include <string>

class GLShader
{
private:
    std::string GLSL;
    GLuint mProgram;
    void AttachGLSL(std::string glsl_file_path, GLenum type);

public:
    GLShader(std::string glsl_file_path, bool load_geometry = false);
    GLuint GetShaderProgram();
    void SetUniform(const std::string &name, int value);
    void SetUniform(const std::string &name, float value);
    void SetUniform(const std::string &name, bool value);
    void SetUniform(const std::string &name, glm::mat4 mat4);
    void SetUniform(const std::string &name, glm::mat3 mat3);
    void SetUniform(const std::string &name, glm::vec3 vec3);
    void Use()
    {
        glUseProgram(mProgram);
    }
    ~GLShader();
};


enum class CameraMovement
{
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
};

constexpr float YAW = -90.0f;
constexpr float PITCH = 0.0f;
constexpr float SPEED = 2.5f;
constexpr float SENSITIVITY = 0.05f;
constexpr float ZOOM = 45.0f;

class EulerCamera
{
private:
    // camera Attributes
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;
    // euler Angles
    float m_yaw;
    float m_pitch;
    // camera options
    float m_speed;
    float m_mouseSensitivity;
    float m_zoom;
    // calculates the front vector from the Camera's (updated) Euler Angles
    void UpdateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front.y = sin(glm::radians(m_pitch));
        front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        m_right = glm::normalize(glm::cross(m_front, m_worldUp)); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        m_up = glm::normalize(glm::cross(m_right, m_front));
    }

public:
    EulerCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f)
        : m_position(position), m_worldUp(up), m_yaw(yaw), m_pitch(pitch), m_speed(SPEED), m_mouseSensitivity(SENSITIVITY), m_zoom(ZOOM)
    {
        UpdateCameraVectors();
    }
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(m_position, m_position + m_front, m_up);
    }
    glm::vec3 GetPosition()
    {
        return m_position;
    }
    glm::vec3 GetFrontVector()
    {
        return m_front;
    }
    glm::mat4 GetProjectionMatrix(GLuint src_width, GLuint src_height)
    {
        return glm::perspective(glm::radians(m_zoom), (float)src_width / (float)src_height, 0.1f, 100.0f);
    }
    float GetZoom()
    {
        return m_zoom;
    }
    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessDirection(CameraMovement direction, float deltaTime)
    {
        float cameraSpeed = static_cast<float>(2.5 * deltaTime);
        if (direction == CameraMovement::Forward)
            m_position += cameraSpeed * m_front;
        if (direction == CameraMovement::Backward)
            m_position -= cameraSpeed * m_front;
        if (direction == CameraMovement::Left)
            m_position -= m_right * cameraSpeed;
        if (direction == CameraMovement::Right)
            m_position += m_right * cameraSpeed;
        if (direction == CameraMovement::Up)
            m_position += m_up * cameraSpeed;
        if (direction == CameraMovement::Down)
            m_position -= m_up * cameraSpeed;
    }
    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= m_mouseSensitivity;
        yoffset *= m_mouseSensitivity;

        m_yaw += xoffset;
        m_pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (m_pitch > 89.0f)
                m_pitch = 89.0f;
            if (m_pitch < -89.0f)
                m_pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        UpdateCameraVectors();
    }
    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        m_zoom -= (float)yoffset;
        if (m_zoom < 1.0f)
            m_zoom = 1.0f;
        // if (m_zoom > 65.0f)
        //     m_zoom = 65.0f;
    }
};

int GlfwGladInitialization(GLFWwindow **window, int SRC_WIDTH, int SRC_HEIGHT, const char *title);
void FramebufferSizeCallback(GLFWwindow *window, int width, int height);
GLuint LoadTexture(const char *file_path, GLint mode = GL_REPEAT, bool gamma = false);

void RenderSphere();
void RenderQuad();