#include <iostream>
#include <pointing/pointing.h>
#include <fstream>
#include <stdio.h>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <math.h>

using namespace pointing;

TransferFunction *func = 0;

// context is user data, timestamp is a moment at which the event was received
// input_dx, input_dy are displacements in horizontal and vertical directions
// buttons is a variable indicating which buttons of the pointing device were pressed.

std::ofstream outfile;

time_t _tm =time(NULL );

struct tm * curtime = localtime ( &_tm );

std::string filename = std::string(asctime(curtime)) + std::string("_output.csv");

// create a name for the file output

void pointingCallback(void *, TimeStamp::inttime timestamp, int input_dx, int input_dy, int buttons) {
    if (!func) return;

    int output_dx = 0, output_dy = 0;
    // In order to use a particular transfer function, its applyi method must be called.
    func->applyi(input_dx, input_dy, &output_dx, &output_dy, timestamp);
    
    int dx = output_dx - input_dx;
    int dy = output_dy - input_dy;
    
    // TODO: D -> Distance from current position to target position
    // TODO: In this code, speed is the speed relative to the position of the virtual cursor(Question: dx, dy are pixel? or relative coordinate?). so, we need to discuss about 'speed'
    
    outfile.open(filename, std::ios_base::app);
    outfile << timestamp << "," << dx << "," << dy << "," << "D" << "," << sqrt (dx*dx + dy*dy)<< "\n";
    outfile.close();
    
//    std::cout << "Displacements in x and y: " << input_dx << " " << input_dy << std::endl;
//    std::cout << "Corresponding pixel displacements: " << output_dx << " " << output_dy << std::endl;
    
    
    
}


int main() {
    // Basically, to start using main functionality of libpointing
    // one needs to create objects of PointingDevice, DisplayDevice classes,
    // connect them passing to TransferFunction class object.
    
    outfile.open(filename, std::ios_base::app);
    outfile << "timestamp,dx,dy,D,speed" << "\n";
    outfile.close();
    
    // Any available pointing and display devices
    // if debugLevel > 0, the list of available devices
    // and extended information will be output.
    PointingDevice *input = PointingDevice::create("any:?debugLevel=1");
    DisplayDevice *output = DisplayDevice::create("any:?debugLevel=1");

    func = TransferFunction::create("sigmoid:?debugLevel=2", input, output);

    // To receive events from PointingDevice object, a callback function must be set.
    input->setPointingCallback(pointingCallback);
    
    while (1)
        PointingDevice::idle(100); // milliseconds
        
    
    
    delete input;
    delete output;
    delete func;

    return 0;
}
