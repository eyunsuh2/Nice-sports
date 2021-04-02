#include <pointing/pointing.h>
#include <iomanip>
#include <stdexcept>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "shader.h"
#include <Windows.h>

#include <iostream>

using namespace pointing;
using namespace std;
TransferFunction* func = 0;
TimeStamp::inttime last_time = 0;
bool button_pressed = false;


// Function Prototypes
GLFWwindow* glAllInit();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void render();

// Global variables
GLFWwindow* window = NULL;
Shader* globalShader = NULL;
unsigned int SCR_WIDTH = 600;                                           // 창크기
unsigned int SCR_HEIGHT = 600;
double xpos, ypos;
unsigned int VBO, VAO, EBO;
float vertices[] = {
    // positions
    0.5f,  0.5f, 0.0f, // top right
    0.5f, -0.5f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f, // bottom left
    -0.5f,  0.5f, 0.0f  // top left
};
unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};


void
pointingCallback(void* /*context*/, TimeStamp::inttime timestamp,
    int input_dx, int input_dy, int buttons) {
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
    window = glAllInit();

    // shader loading and compile (by calling the constructor)
    globalShader = new Shader("4.1.transform.vs", "4.1.transform.fs");

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    if (argc < 3)
        std::cout
        << "Usage: " << argv[0] << " [inputdeviceURI [outputdeviceURI [transferfunctionURI]]]" << std::endl
        << "Using default values for some parameters" << std::endl;

    // --- Pointing device ----------------------------------------------------

    PointingDevice* input = PointingDevice::create(argv[1]); //  ----------------------------------------------------- 무슨 Pointing Device를 쓰느냐를 정하는 곳
                                                             //  ----------------------------------------------------- 이거를 원래는 ID로 해야되는데 ID를 모를 가능성이 높음. 그래서 argv 값 바꿔가면서 이것저것 해보기.
    // PointingDevice *input = PointingDevice::create(argc>1?argv[1]:"any:?debugLevel=1"); // -------------------- 잘 안되면 이거로 해보셈
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

    // --- Display device -----------------------------------------------------

    DisplayDevice* output = DisplayDevice::create(argc > 2 ? argv[2] : "windisplay:/PHLC208?bw=1920&bh=1080&w=598&h=336&hz=60"); // ---------- 무슨 Display Device를 쓰느냐를 정하는 곳
    // DisplayDevice *output = DisplayDevice::create(argc>2?argv[2]:"any:?debugLevel=1") ; // -------------------- 잘 안되면 이거로 해보셈

    double hdpi, vdpi;
    output->getResolution(&hdpi, &vdpi);
    DisplayDevice::Size size = output->getSize();
    DisplayDevice::Bounds bounds = output->getBounds(); // 1920 x 1080 pixel
    std::cout << std::endl << "Display device" << std::endl
        << "  " << output->getURI(true).asString() << std::endl
        << "  " << bounds.size.width << " x " << bounds.size.height << " pixels, "
        << size.width << " x " << size.height << " mm" << std::endl
        << "  " << hdpi << " x " << vdpi << " PPI, "
        << output->getRefreshRate() << " Hz" << std::endl;

    // --- Transfer function --------------------------------------------------

    func = TransferFunction::create(argc > 3 ? argv[3] : "naive:?cdgain=1", input, output); // function setting 

    std::cout << std::endl << "Transfer function" << std::endl
        << "  " << func->getURI(true).asString() << std::endl
        << std::endl;

    // render loop
    // -----------
    input->setPointingCallback(pointingCallback);

    glfwGetCursorPos(window, &xpos, &ypos);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        PointingDevice::idle(0.01);
        render();
    }


    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;

}

GLFWwindow* glAllInit()
{
    GLFWwindow* window;

    // glfw: initialize and configure
    if (!glfwInit()) {
        printf("GLFW initialisation failed!");
        glfwTerminate();
        exit(-1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // glfw window creation
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Transformation", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    // Allow modern extension features
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        cout << "GLEW initialisation failed!" << endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(-1);
    }

    return window;
}

void render()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // create transformations
    glm::mat4 transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    transform = glm::translate(transform, glm::vec3( (xpos-((SCR_WIDTH)/2))/ ((SCR_WIDTH) / 2), -(ypos-(SCR_HEIGHT/2))/ ((SCR_WIDTH) / 2), 0.0f));                 // 창 크기에 맞춰져 세팅되어있음.
    transform = glm::scale(transform, glm::vec3(0.1f, 0.1f, 0.0f));

    // get matrix's uniform location and set matrix
    globalShader->use();
    unsigned int transformLoc = glGetUniformLocation(globalShader->ID, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

    // render
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        glfwGetCursorPos(window, &xpos, &ypos);
    }
}
