#!/bin/sh

g++ rplidar_interface.cpp -L lib/rplidar -lrplidar_sdk_laptop -lstdc++ -pthread
