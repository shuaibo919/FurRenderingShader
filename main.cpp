#include "utils.h"

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
EulerCamera camera(glm::vec3(0.0f, 0.0f, 1.f));

bool firstMouse = true;
void MouseCallback(GLFWwindow *window, double xposIn, double yposIn);
void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);

int main()
{
    GLFWwindow *window = nullptr;
    if (GlfwGladInitialization(&window, SCR_WIDTH, SCR_HEIGHT, "FurRenderingShader") == -1)
    {
        return -1;
    }
    glfwSwapInterval(1);

    // 设置回调函数
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, MouseScrollCallback);

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    // Setup some OpenGL options
    glEnable(GL_DEPTH_TEST);

    // Setup and compile our shaders
    GLuint diffuseTex = LoadTexture("Resource/fur_color.jpg");
    GLuint noiseTex = LoadTexture("Resource/FurPattern_05_v2.png");
    GLShader shaderGeometryPass("Resource/g_buffer_fur");
    GLShader shaderLightingPass("Resource/lightpass_fur");
    GLShader shaderBasePass("Resource/g_buffer_fur_stencil");

    // Set samplers
    shaderLightingPass.Use();
    shaderLightingPass.SetUniform("gPosition", 0);
    shaderLightingPass.SetUniform("gNormal", 1);
    shaderLightingPass.SetUniform("gAlbedoSpec", 2);

    // Models
    glm::vec3 objectPos = glm::vec3(0,0,0);
    glm::vec3 lightPos = glm::vec3(0.0f, 2.0f, 2.0f);

    // Set up Stencil-Buffer
    GLuint gBufferStencil;
    GLuint gPositionStencil;
    {
        glGenFramebuffers(1, &gBufferStencil);
        glBindFramebuffer(GL_FRAMEBUFFER, gBufferStencil);
        glGenTextures(1, &gPositionStencil);
        glBindTexture(GL_TEXTURE_2D, gPositionStencil);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPositionStencil, 0);
        GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, attachments);
        GLuint rboDepth;
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Framebuffer not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Set up G-Buffer
    GLuint gBuffer;
    GLuint gPosition, gNormal, gAlbedoSpec;
    {
        glGenFramebuffers(1, &gBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        // - Position color buffer
        glGenTextures(1, &gPosition);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
        // - Normal color buffer
        glGenTextures(1, &gNormal);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
        // - Color + Specular color buffer
        glGenTextures(1, &gAlbedoSpec);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
        // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
        GLuint attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, attachments);
        // - Create and attach depth buffer (renderbuffer)
        GLuint rboDepth;
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
        // - Finally check if framebuffer is complete
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Framebuffer not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }
        glfwPollEvents();

        {
            // 1. Fur Base Pass
            glBindFramebuffer(GL_FRAMEBUFFER, gBufferStencil);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glm::mat4 projection = camera.GetProjectionMatrix(SCR_WIDTH, SCR_HEIGHT);
            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 model = glm::mat4(1.0f);
            shaderBasePass.Use();
            shaderBasePass.SetUniform("projection", projection);
            shaderBasePass.SetUniform("view", view);
            model = glm::translate(model, objectPos);
            model = glm::scale(model, glm::vec3(0.225f));
            shaderBasePass.SetUniform("model", model);
            RenderSphere();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        {
            // 2. Geometry Pass
            glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuseTex);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, noiseTex);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, gPositionStencil);
            glm::mat4 projection = camera.GetProjectionMatrix(SCR_WIDTH, SCR_HEIGHT);
            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 model = glm::mat4(1.0f);
            shaderGeometryPass.Use();
            shaderGeometryPass.SetUniform("projection", projection);
            shaderGeometryPass.SetUniform("view", view);
            shaderGeometryPass.SetUniform("texture_diffuse", 0);
            shaderGeometryPass.SetUniform("texture_noise", 1);
            shaderGeometryPass.SetUniform("texture_basePosition", 2);

            model = glm::translate(model, objectPos);
            model = glm::scale(model, glm::vec3(0.25f));
            shaderGeometryPass.SetUniform("model", model);
            shaderGeometryPass.SetUniform("viewPos", camera.GetPosition());
            RenderSphere();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        {
            // 3. Lighting Pass
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            shaderLightingPass.Use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gPosition);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gNormal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
            shaderLightingPass.SetUniform("gPosition", 0);
            shaderLightingPass.SetUniform("gNormal", 1);
            shaderLightingPass.SetUniform("gAlbedoSpec", 2);
            shaderLightingPass.SetUniform("lightPos", lightPos);
            shaderLightingPass.SetUniform("viewPos", camera.GetPosition());
            // Finally render quad
            RenderQuad();
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void MouseCallback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if ((glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
        camera.ProcessMouseMovement(xoffset / 10.0f, yoffset / 10.0f);
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS))
    {
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;
        camera.ProcessDirection(CameraMovement::Left, xoffset * deltaTime);
        camera.ProcessDirection(CameraMovement::Down, yoffset * deltaTime);
        lastX = xpos;
        lastY = ypos;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        firstMouse = true;
    }
}

void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
