#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>
#include <string>
#include <vector>

#include "PSX.h"
#include "Constants.h"
#include "Shader.h"

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ProcessInput(GLFWwindow* window);

int main(int argc, char** argv) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(VRAM_WIDTH, VRAM_HEIGHT, "PSX Emulator", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window);
    glViewport(0, 0, VRAM_WIDTH, VRAM_HEIGHT);
    Shader shader("VertexShader.glsl", "FragmentShader.glsl");
    PSX system;

    const uint16_t* data = system.GetVRAM().data();
    // create and bind texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, (const void*)data);
    glGenerateMipmap(GL_TEXTURE_2D);

    float vertices[] = {
        -1.0f,  1.0f,  0.0f,
        -1.0f, -1.0f,  0.0f,
         1.0f,  1.0f,  0.0f,
         1.0f, -1.0f,  0.0f
    };

    float tex_coords[] = {
         0.0f, 0.0f,
         0.0f, 1.0f,
         1.0f, 0.0f,
         1.0f, 1.0f
    };

    GLuint indices[] = {
        0, 1, 2,
        1, 2, 3
    };

    GLuint VBO, VAO, VBO2, EBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &VBO2);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tex_coords), tex_coords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    int cycles = 1;
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    while (!glfwWindowShouldClose(window)) {
        ProcessInput(window);
        system.RunFrame();
        //system.DumpRAM();
        glfwSwapBuffers(window);
        data = system.GetVRAM().data();
        glBindVertexArray(VAO);
        shader.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, VRAM_WIDTH, VRAM_HEIGHT, GL_RGBA,
            GL_UNSIGNED_SHORT_1_5_5_5_REV, (const void*)data);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        cycles++;
        
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void ProcessInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}