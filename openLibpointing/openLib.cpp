#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <math.h>
#include <cstdlib>
#include <shader.h>
#include <pointing/pointing.h>
#include <iomanip>
#include <stdexcept>
#include <Windows.h>
#include <stdio.h>
#include <ctime>
#include <string>
#include <chrono>

using namespace pointing;

void update_dot_vertex(int vIndex, float x, float y);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

TransferFunction* func = 0;
const double err = -2 ^ 31;
std::chrono::system_clock::time_point clock_start;
std::chrono::minutes duration;
std::chrono::milliseconds csv_timestamp;

// other functions
void update_dot_vertex(int vIndex, float x, float y);
void render(GLFWwindow* window);


// settings
unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;
unsigned int VBO[3], VAO[3], EBO; // 0: circle, 1: line, 2:point
float Cursors[] = {
    0.0f, 0.0f, 0.0f,
    0.0f, -0.918f, 0.0f,
    0.207f,-0.7226f, 0.0f,
    0.328f, -1.0f, 0.0f,
    0.514f, -0.9141f, 0.0f,
    0.395f, -0.6387f, 0.0f,
    0.6759f, -0.623f, 0.0f
};
float Targets[1083];
float Target_x = 0, Target_y = 0;
float finalx1 = 0;
float finaly1 = 0;
float r = 0.1f;
float cursorSize = 0.0025f;
float width_value = 9;
float height_value = 16;
float total_value = width_value + height_value;
float cursor_width = width_value * cursorSize;
float cursor_height = height_value * cursorSize;
int total_click = 0, correct_click = 0;

std::string mouse_function = "system:?slider=0&epp=false";     // function setting - constant
std::string mouse_function_name = "con";
//std::string mouse_function = "system:?slider=0&epp=true"; // function setting - acceleration
//std::string mouse_function_name = "acc";

float accuracy = 0.04;
int nOfPoints = 1;
Shader* DotShader, * globalShader;
double xpos, ypos;
bool start = false;

std::ofstream outfile;
time_t _tm = time(NULL);
#pragma warning(suppress : 4996)
struct tm* curtime = localtime(&_tm);
#pragma warning(suppress : 4996)
std::string filename = std::to_string(curtime->tm_year + 1900) + std::string("_") + std::to_string(curtime->tm_mon + 1) + std::string("_") + std::to_string(curtime->tm_mday)
+ std::string("_") + std::to_string(curtime->tm_hour) + std::string("_") + std::to_string(curtime->tm_min) + std::string("_") + std::to_string(curtime->tm_sec) + std::string("_") + mouse_function_name
+ std::string("_output.csv");



void pointingCallback(void*, TimeStamp::inttime timestamp, int input_dx, int input_dy, int buttons) {
    if (!func) return;

    int output_dx = 0, output_dy = 0;
    // In order to use a particular transfer function, its applyi method must be called.
    func->applyi(input_dx, input_dy, &output_dx, &output_dy, timestamp);
    xpos += output_dx;
    ypos += output_dy;

    csv_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - clock_start);
    if (start) {
        int dx = input_dx; // 마우스 x속도
        int dy = input_dy; // 마우스 y속도
        outfile << csv_timestamp.count() << "," << dx << "," << dy << "," << sqrt(pow((Targets[0] - Cursors[0])*960, 2) + pow((Targets[1] - Cursors[1])*540, 2)) << "," << sqrt(dx * dx + dy * dy) << "," << r*345.6 << "," 
            << total_click << "," << correct_click << '\n';
    }

    // create transformations
    Cursors[0] = (xpos - ((SCR_WIDTH) / 2)) / ((SCR_WIDTH) / 2); // Cursor x좌표
    Cursors[1] = -(ypos - (SCR_HEIGHT / 2)) / ((SCR_WIDTH) / 2); // Cursor y좌표
    Cursors[3] = Cursors[0];
    Cursors[4] = Cursors[1] - (0.918f * cursor_height);
    Cursors[6] = Cursors[0] + (0.207f * cursor_width);
    Cursors[7] = Cursors[1] - (0.7226f * cursor_height);
    Cursors[9] = Cursors[0] + (0.328f * cursor_width);
    Cursors[10] = Cursors[1] - (1.0f * cursor_height);
    Cursors[12] = Cursors[0] + (0.514f * cursor_width);
    Cursors[13] = Cursors[1] - (0.9141f * cursor_height);
    Cursors[15] = Cursors[0] + (0.395f * cursor_width);
    Cursors[16] = Cursors[1] - (0.6387f * cursor_height);
    Cursors[18] = Cursors[0] + (0.6759f * cursor_width);
    Cursors[19] = Cursors[1] - (0.623f * cursor_height);

}



int main(int argc, char** argv)
{
    std::cout << filename;
    outfile.open(filename);
    outfile << "timestamp,dx,dy,D,speed,radius,clicks,correct" << "\n";
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Libpointing", glfwGetPrimaryMonitor(), NULL); //glfwGetPrimaryMonitor(), NULL);
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


    PointingDevice* input = PointingDevice::create(argv[1]);

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
    func = TransferFunction::create(argc > 3 ? argv[3] : mouse_function, input, output);
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

    // Target Vertices 설정
    for (int i = 0; i < 361; i++) {
        float angle = i * 3.1415926 / 180;
        float a1 = r * cos(angle) * width_value / total_value;
        float a2 = r * sin(angle) * height_value / total_value;
        Targets[3 * i] = a1;
        Targets[3 * i + 1] = a2;
        Targets[3 * i + 2] = 0.0f;
    }

    for (int i = 0; i < 21; i++) {
        if (i % 3 == 0) {
            Cursors[i] *= cursor_width;
        }
        else {
            Cursors[i] *= cursor_height;
        }
    }

    while (!glfwWindowShouldClose(window))
    {
        PointingDevice::idle(0.01);
        render(window);
        glfwPollEvents();
    }
    outfile.close();

    glDeleteVertexArrays(3, VAO);
    glDeleteBuffers(3, VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void render(GLFWwindow* window) {

    if (start) {
        duration = std::chrono::duration_cast<std::chrono::minutes>(
            std::chrono::system_clock::now() - clock_start);
        if (duration.count() == 15) {
            glfwSetWindowShouldClose(window, true);
        }
    }
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);


    // 화면에 점 출력
    if (nOfPoints == 1) {
        glBindVertexArray(VAO[2]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Targets), Targets, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        DotShader->use();
        glBindVertexArray(VAO[2]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 361);
        glBindVertexArray(0);
    }

    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Cursors), Cursors, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // render
    globalShader->use();
    glBindVertexArray(VAO[0]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 7);
    glBindVertexArray(0);

    glfwSwapBuffers(window);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    float nx, ny;
    total_click++;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        nx = Cursors[0]; ny = Cursors[1];

        if (pow(nx-Target_x,2) * pow(total_value / width_value,2) + pow(ny-Target_y, 2) * pow(total_value / height_value,2) <= pow(r,2)) {
            finalx1 = (((float)rand() / 20479) - 0.8);
            finaly1 = (((float)rand() / 20479) - 0.8);
            update_dot_vertex(0, finalx1, finaly1);
            correct_click++;
        }
    }

}

void update_dot_vertex(int vIndex, float x, float y)
{
    Target_x = x; Target_y = y;

    int random_number = rand();
    if (random_number % 4 == 0) {
        r = 0.1f;
    }
    else if (random_number % 4 == 1) {
        r = 0.133f;
    }
    else if (random_number % 4 == 2) {
        r = 0.166f;
    }
    else {
        r = 0.2f;
    }

    // Target Vertices 설정
    for (int i = 0; i < 361; i++) {
        float angle = i * 3.1415926 / 180;
        float a1 = r * cos(angle) * width_value / total_value;
        float a2 = r * sin(angle) * height_value / total_value;
        Targets[3 * i] = x + a1;
        Targets[3 * i + 1] = y + a2;
        Targets[3 * i + 2] = 0.0f;
    }

    if (!start) {
        start = true;
        clock_start = std::chrono::system_clock::now();
    }

    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Targets), Targets, GL_STATIC_DRAW);
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