#!/bin/sh

g++ komMain.cpp komhub.cpp rplidar_interface.cpp RobotMap.cpp CommServer.cpp linefinder/Linefinder.cpp linefinder/HelperClasses.cpp -lwiringPi -L lib/rplidar -lrplidar_sdk_rpi -lstdc++ -pthread
