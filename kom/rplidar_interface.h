#ifndef RPLIDAR_INTERFACE_H
#define RPLIDAR_INTERFACE_H

#include <iostream>
#include "lib/rplidar/rplidar.h"
#include "linefinder/HelperClasses.h"
#include <stdexcept>

using namespace std;
using namespace rp::standalone::rplidar;

// Creates an RPlidarDriver and connects it to "/dev/ttyUSBX" where X is the port.
// If no port is specified, it tires to connect with all port 0-9.
RPlidarDriver* initLidar(int port = -1);

// Starts the lidar motor and starts the scans.
void startLidar(RPlidarDriver* lidar);

// Stops the motor and stops scanning the sourindings.
void stopLidar(RPlidarDriver* lidar);

// Disconnects the lidar from the port and disposes the object.
void disconnectLidar(RPlidarDriver *lidar);

// Gets the scan from the latest revolution and puts it as points into vec.
// The points have cartesian coordinates and are in millimeters.
void getLidarScan(RPlidarDriver* lidar, vector<Point>* vec);

#endif // RPLIDAR_INTERFACE_H
