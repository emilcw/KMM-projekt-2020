#include <iostream>
#include "rplidar_interface.h"
#include <math.h>
#include <string.h>

bool isConnected = false;

bool connectLidar(RPlidarDriver *lidar, int port) {
    u_result res;
    char path[13] = "/dev/ttyUSB0";
    path[11] = (char)(48 + port);

    res = lidar->connect(path, 115200);
    if (IS_OK(res)) {
        isConnected = true;
        cout << "Connected to LIDAR on port /dev/ttyUSB" << port << endl;

        rplidar_response_device_health_t health;
        res = lidar->getHealth(health);
        if (IS_OK(res)) {
            cout << "Lidar health: " << health.status << endl;
        }
    }
}

RPlidarDriver* initLidar(int port) {
    RPlidarDriver* lidar = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);
    if (!lidar) {
        cout << "Lidar could not be created. Insufficent memory?" << endl;
        return nullptr;
    }

    if (port == -1) {
        int i = 0;
        while (!isConnected && i < 10) {
            connectLidar(lidar, i);
            i++;
        }
    } else connectLidar(lidar, port);

    if (isConnected) {
        return lidar;
    } else {
        cout << "Lidar could NOT be connected" << endl;
        return nullptr;
    }
}

void startLidar(RPlidarDriver *lidar) {
    if (!isConnected) return;
    lidar->startMotor();
    cout << "Motor has started" << endl;
    lidar->startScan(false, true);
}

void stopLidar(RPlidarDriver *lidar) {
    if (!isConnected) return;
    lidar->stop();
    lidar->stopMotor();

}

void disconnectLidar(RPlidarDriver *lidar) {
    if (!isConnected) return;
    lidar->disconnect();
    RPlidarDriver::DisposeDriver(lidar);
    isConnected = false;

}

void getLidarScan(RPlidarDriver *lidar, vector<Point> *vec) {
    if (!isConnected) return;
    while (true) {
        size_t count = 2048;
        rplidar_response_measurement_node_hq_t nodes[count];
        
        u_result res = lidar->grabScanDataHq(nodes, count);
        if (IS_OK(res)) {
            // Sorts the data by increasing angle
            lidar->ascendScanData(nodes, count);
            vec->reserve(count);
            for (size_t i = 0; i < count; i++) {
                // Calculates the angle in radians and distance in meters.
                float angle = (nodes[i].angle_z_q14 * 90.f / (1 << 14)) * 3.141592/180;
                float distance = nodes[i].dist_mm_q2 / 1000.f / (1 << 2);

                // Saves all points that are more than 3cm away.
                if (distance > 0.03) {
                    int x = int(distance * (-sin(angle)) * 1000);
                    int y = int(distance * (-cos(angle)) * 1000);
                    vec->emplace_back(Point(x, y));
                }
            }
            break;
        }
    }
}
    
/*
int main() {
    
    RPlidarDriver* lidar;
    lidar = initLidar();

    startLidar(lidar);
    
    vector<Point> scans;
    getLidarScan(lidar, &scans);
    for (int i = 0; i < scans.size(); i++) {
        cout << scans.at(i).x;
        cout << " ";
        cout << scans.at(i).y << endl;
    }
        
    stopLidar(lidar);
    disconnectLidar(lidar);
    return 0;
}*/
