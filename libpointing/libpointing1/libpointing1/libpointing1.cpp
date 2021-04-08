// 09_ShaderClass
//   - Shader setup process simplified using Shader class
//   - Color interpolation
//   - ESC to quit

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include <cstdlib>
#include <ctime>
#include <shader.h>
#include <pointing/pointing.h>

using namespace pointing;

void update_dot_vertex(int vIndex, float x, float y);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

TransferFunction* func = 0;
const double err = -2 ^ 31;


// other functions
void normalize_cursor_position(double x, double y, float& nx, float& ny);
//void update_dot_vertex(int vIndex, float x, float y);
void calcDots();
void render(GLFWwindow* window);


// settings
unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;
float numOFver = 900;
float circleVertex[900];
float PI = 3.14159265358979323846;
bool finished = false;
unsigned int VBO[3], VAO[3]; // 0: circle, 1: line, 2:point
float Lines[6];
float Dots[6];
float finalx1 = 0;
float finaly1 = 0;
float accuracy = 0.04;
int nOfPoints = 1;
int numOfDots = 0;
bool createdVAO = false;
Shader CircleShader, LineShader, DotShader;


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



int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Libpointing", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Allow modern extension features
    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK)
    {
        printf("GLEW initialisation failed!");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }


    PointingDevice* input = PointingDevice::create("any:?debugLevel=1");
    DisplayDevice* output = DisplayDevice::create("any:?debugLevel=1");

    func = TransferFunction::create("sigmoid:?debugLevel=2", input, output);


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    CircleShader.initShader("circleVertexShader.vs", "circleFragShader.fs"); // you can name your shader files however you like
    LineShader.initShader("lineVertexShader.vs", "lineFragShader.fs");
    DotShader.initShader("dotVertexShader.vs", "dotFragShader.fs");



    for (int i = 0; i < numOFver; i += 3) {
        circleVertex[i] = 0.5 * (cos(2.0 * i * PI / numOFver));
        circleVertex[i + 1] = 0.5 * (sin(2.0 * i * PI / numOFver));
        circleVertex[i + 2] = 0.0f;
    }

    glGenVertexArrays(3, VAO);
    glGenBuffers(3, VBO);
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertex), circleVertex, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ==========================================

    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Lines), Lines, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //=========================================

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    std::srand((unsigned int)time(NULL));

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        render(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(3, VAO);
    glDeleteBuffers(3, VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void render(GLFWwindow* window) {

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);


    // 화면에 점 출력
    if (nOfPoints == 1) {
        glPointSize(30.0);
        DotShader.use();
        glBindVertexArray(VAO[2]);
        glDrawArrays(GL_POINTS, 0, 1);
    }
    glfwSwapBuffers(window);
}


void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    float cx, cy;
    normalize_cursor_position(xpos, ypos, cx, cy);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    double xpos, ypos;
    float nx, ny;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        glfwGetCursorPos(window, &xpos, &ypos);
        normalize_cursor_position(xpos, ypos, nx, ny);
        
        std::cout << "(x,y) coordinates are: " << nx << " " << ny << std::endl;
        
        if (nx <= finalx1 + accuracy and nx >= finalx1 - accuracy) {
            if (ny <= finaly1 + accuracy and ny >= finaly1 - accuracy) {
                finalx1 = (((float)rand() / 16383) - 1);
                finaly1 = (((float)rand() / 16383) - 1);
                update_dot_vertex(0, finalx1, finaly1);
            }
        }
    }
   
}

void update_dot_vertex(int vIndex, float x, float y)
{
    float n[3];
    n[0] = x;
    n[1] = y;
    n[2] = 0.0f;
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferSubData(GL_ARRAY_BUFFER, vIndex * sizeof(float) * 3, sizeof(float) * 3, n);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    Dots[0] = x;
    Dots[1] = y;
    Dots[2] = 0.0f;

    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Dots), Dots, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}



void normalize_cursor_position(double x, double y, float& nx, float& ny)
{
    nx = ((float)x / (float)SCR_WIDTH) * 2.0f - 1.0f;
    ny = -1.0f * (((float)y / (float)SCR_HEIGHT) * 2.0f - 1.0f);
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

