#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <math.h>
#include <cstdlib>
#include <ctime>
#include <shader.h>
#include <pointing/pointing.h>
#include <iomanip>
#include <stdexcept>
#include <Windows.h>

using namespace pointing;

void update_dot_vertex(int vIndex, float x, float y);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
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
unsigned int VBO[3], VAO[3], EBO; // 0: circle, 1: line, 2:point
float Dots[6]; float Cursors[6];
float finalx1 = 0;
float finaly1 = 0;
float accuracy = 0.04;
int nOfPoints = 1;
Shader *DotShader, *globalShader;
double xpos, ypos;

static float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.5f,  0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f
};
unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};


void pointingCallback(void*, TimeStamp::inttime timestamp, int input_dx, int input_dy, int buttons) {
    if (!func) return;

    int output_dx = 0, output_dy = 0;
    // In order to use a particular transfer function, its applyi method must be called.
    func->applyi(input_dx, input_dy, &output_dx, &output_dy, timestamp);
    xpos += output_dx;
    ypos += output_dy;
}



int main(int argc, char** argv)
{
    ShowCursor(false);

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Libpointing", glfwGetPrimaryMonitor(), NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
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


    PointingDevice *input = PointingDevice::create("winhid:/186911001");

    for (TimeStamp reftime, now;
        !input->isActive() && now - reftime < 15 * TimeStamp::one_second;
        now.refresh())
        PointingDevice::idle(500);
    std::cout << std::endl << "Pointing device" << std::endl;
    std::cout << "  " << input->getURI(true).asString() << std::endl
        << "  " << input->getResolution() << " CPI, "
        << input->getUpdateFrequency() << " Hz" << std::endl
        << "  device is " << (input->isActive() ? "" : "not ") << "active" << std::endl
        << std::endl;
    DisplayDevice* output = DisplayDevice::create(argc > 2 ? argv[2] : "windisplay:/PHLC208?bw=1920&bh=1080&w=598&h=336&hz=60");
    func = TransferFunction::create(argc > 3 ? argv[3] : "naive:?cdgain=1", input, output); // function setting 
    input->setPointingCallback(pointingCallback);


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    globalShader = new Shader("4.1.transform.vs", "4.1.transform.fs");
    DotShader = new Shader("dotVertexShader.vs", "dotFragShader.fs");
    glGenVertexArrays(3, VAO);
    glGenBuffers(3, VBO);

    // ==========================================

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.


    std::srand((unsigned int)time(NULL));

    // render loop
    // -----------

    while (!glfwWindowShouldClose(window))
    {
        PointingDevice::idle(0.01);
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

    // create transformations
    Cursors[0] = (xpos - ((SCR_WIDTH) / 2)) / ((SCR_WIDTH) / 2);
    Cursors[1] = -(ypos - (SCR_HEIGHT / 2)) / ((SCR_WIDTH) / 2);

    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Cursors), Cursors, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // render
    glPointSize(20.0);
    glBindVertexArray(VAO[0]);
    glDrawArrays(GL_POINTS, 0, 1);
    glBindVertexArray(0);

    // 화면에 점 출력
    if (nOfPoints == 1) {
        glPointSize(30.0);
        DotShader->use();
        glBindVertexArray(VAO[2]);
        glDrawArrays(GL_POINTS, 0, 1);
        glBindVertexArray(0);
    }
    glfwSwapBuffers(window);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    float nx, ny;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        nx = Cursors[0]; ny = Cursors[1];

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
