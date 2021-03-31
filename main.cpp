#include <iostream>
#include <pointing/pointing.h>
#include <stdio.h>
#include <stdlib.h>
#include <shader.h>

#include <GLFW/glfw3.h>
#include <GL/glew.h>

using namespace pointing;
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

bool verticalFlip = false;
float colorR = 1.0f, colorG = 0.0f, colorB = 0.0f;
float mov_x = 0.0f, mov_y = 0.0f;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action , int mods);

const double err = -2^31;



TransferFunction *func = 0;

// context is user data, timestamp is a moment at which the event was received
// input_dx, input_dy are displacements in horizontal and vertical directions
// buttons is a variable indicating which buttons of the pointing device were pressed.
void pointingCallback(void *, TimeStamp::inttime timestamp, int input_dx, int input_dy, int buttons) {
    if (!func) return;

    int output_dx = 0, output_dy = 0;
    // In order to use a particular transfer function, its applyi method must be called.
    func->applyi(input_dx, input_dy, &output_dx, &output_dy, timestamp);
    if(output_dx > err && output_dy > err) {
        std::cout << "Displacements in x and y: " << input_dx << " " << input_dy << std::endl;
        std::cout << "Corresponding pixel displacements: " << output_dx << " " << output_dy << std::endl;
        std::cout << "tiemstamp " << timestamp << std::endl;
        
    }
    
    
}

int main() {
    
    PointingDevice *input = PointingDevice::create("any:?debugLevel=1");
    DisplayDevice *output = DisplayDevice::create("any:?debugLevel=1");

    func = TransferFunction::create("sigmoid:?debugLevel=2", input, output);

    // To receive events from PointingDevice object, a callback function must be set.
    input->setPointingCallback(pointingCallback);

    
    if (!glfwInit())
    {
        printf("GLFW initialisation failed!");
        glfwTerminate();
        return 1;
    }
    
    
    // 4 AA
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LibPointing", NULL, NULL);
    
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("GLEW initialisation failed!");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    
    // gl
    Shader ourShader("2.4.shader.vs", "2.4.shader.fs");

    float vertices[] = {
        -0.2f,  -0.2f, 0.0f,
         0.2f,  -0.2f, 0.0f,
         0.0f,  0.2f, 0.0f
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 2,  // Triangle indices
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // gl
    
    while (!glfwWindowShouldClose(window)) {

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ourShader.setVec4("outColor", colorR, colorG, colorB, 1.0);
//
        ourShader.use();
//
        ourShader.setFloat("multX", mov_x);
        ourShader.setFloat("multY", mov_y);
//
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
//    while (1)
//        PointingDevice::idle(100); // milliseconds

    delete input;
    delete output;
    delete func;

    return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    
    if (key == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        double posX, posY;
        glfwGetCursorPos(window, &posX, &posY);
        std::cout << "(x,y) coordinates are: "<< posX << " " << posY << std::endl;
    }
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    else if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        if (mov_y < 0.7f) mov_y += 0.2f;
    }
    else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        if (mov_x < 0.7f) mov_x += 0.2f;
    }
    else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        if (mov_y > -0.7f) mov_y -= 0.2f;
    }
    else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        if (mov_x > -0.7f) mov_x -= 0.2f;
    }
    
    
    
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}
