#pragma once

#define GL_GLEXT_PROTOTYPES 1
#include <GLFW/glfw3.h>
#include <cstdio>

#define S_HEIGHT 600
#define S_WIDTH 600

void onSetup(int w, int h);
void onDraw(GLFWwindow* window, int fbWidth, int fbHeight);

static void key(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void errorcb(int error, const char* desc) {
    printf("GLFW error %d: %s\n", error, desc);
}

bool glfwLoop(int width = S_WIDTH, int height = S_HEIGHT) {

    GLFWwindow* window;

    if (!glfwInit()) {
        printf("Failed to init GLFW.\n");
        return false;
    }
    glfwSetErrorCallback(errorcb);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(width, height, "alfons", NULL, NULL);
    if (!window) {
        printf("Failed to create window.\n");
        glfwTerminate();
        return false;
    }
    glfwSetKeyCallback(window, key);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetTime(0);
    // double prevt = glfwGetTime();

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

    onSetup(fbWidth, fbHeight);

    while (!glfwWindowShouldClose(window)) {
        double mx, my;
        //double t, dt;
        int winWidth, winHeight;

        // t = glfwGetTime();
        // dt = t - prevt;
        // prevt = t;
        glfwGetCursorPos(window, &mx, &my);
        glfwGetWindowSize(window, &winWidth, &winHeight);
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        glViewport(0, 0, fbWidth, fbHeight);
        glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT |
                GL_STENCIL_BUFFER_BIT);

        onDraw(window, fbWidth, fbHeight);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return true;
}
