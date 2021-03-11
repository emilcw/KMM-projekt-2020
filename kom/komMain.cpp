// kom.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <iostream>
#include <random>
#include <string.h>
#include <unistd.h>
#include "CommServer.h"
#include "komhub.h"
#include "rplidar_interface.h"
#include "RobotMap.h"
#include "linefinder/Linefinder.h"
#include "linefinder/HelperClasses.h"

#define CONST_PI 3.14159265358979323846


using namespace std;

CommServer tcp_server = CommServer();
int lostCount = 0;

void updateMapFromLidar(RPlidarDriver* lidar, float& robotX, float& robotY, float& robotAngle, RobotMap& rmap, int& senMessage)
{
    vector<Point> points;
    getLidarScan(lidar, &points);
    vector<pair<Point*, Point*>> unfilteredLines;
    findAllLines(&points, &unfilteredLines);

    bool offsetUsed = false;
    vector<pair<Point*,Point*>> lines;
    rmap.filterLines(&unfilteredLines, &lines, robotX, robotY, robotAngle);
    //cout << "Found " << unfilteredLines.size() << " unfiltered lines" << endl;
    //cout << "Found " << lines.size() << " lines" << endl;
    if (unfilteredLines.size() == 0 && lines.size() != 0) {
        offsetUsed = true;
        int offsets[] = {2,-2, 4,-4, 6,-6, 8,-8, 10,-10,
                         12,-12, 14,-14, 16,-16, 18,-18, 20,-20,
                         22,-22, 24,-24, 26,-26, 28,-28, 30,-30,
                         32,-32, 34,-34, 36,-36, 38,-38, 40,-40};

        for (int offset : offsets) {
            rmap.filterLines(&unfilteredLines, &lines, robotX, robotY, robotAngle + offset);
            if (lines.size() != 0) {
                robotAngle += offset;
                break;
            }
        }
    }
    
    if (lines.size() == 0) {
        vector<string> command;
        command.push_back("Stay");
        sendToStyr(&command);
        tcp_server.Send("status,Lost myself" + to_string(lostCount++) + ";");
    }
    

    if (!offsetUsed) {
        rmap.updateMap(&lines, robotX, robotY, robotAngle);
        rmap.updatePosition(&lines, &robotX, &robotY, &robotAngle);
    }
    
    sendToSen(COMPARE_ANGLES, robotAngle);
    sendToSen(Commando::REQUEST_ANGLE);
}

void sendUpdateToPC(int goalX, int goalY, float robotX, float robotY, float robotAngle, RobotMap& rmap)
{
    string mapChanges = rmap.getMapChanges();
    //cout << "Updated mapChanges" << endl;
    // Send data to PC
    tcp_server.Send("lidarangle," + to_string(robotAngle) + ";");
    //cout << "Sent stuff 1" << endl;
    tcp_server.Send("move," + to_string(goalX) + "," + to_string(goalY) + ";");
    //cout << "Sent stuff 2" << endl;
    tcp_server.Send("position," + to_string(robotX) + "," + to_string(robotY) + ";");
    //cout << "Sent stuff 5" << endl;
    if (!tcp_server.gethasConnection()) cout << "Lost connection but hasn't noticed" << endl;
    if (mapChanges != "") {
        tcp_server.Send(mapChanges);
        rmap.clearMapChanges();
    }
    //cout << "Sent Mapchanges" << endl;
}

void updateAngle(int goalX, int goalY, float robotX, float robotY, float robotAngle, float& turnAngle)
{
    float diffX = goalX - robotX +0.5;
    float diffY = goalY - robotY +0.5;
    //float travelDistance = hypot(diffX, diffY);

    turnAngle = atan2(diffY, diffX) * 180/CONST_PI;
    if (turnAngle < 0) {
        turnAngle = 360 + turnAngle;
    }
}

bool updatePathfinding(int& goalX, int& goalY, bool& mapComplete, float& robotY, RobotMap& rmap, float& robotX)
{   
    mapComplete = !rmap.findWayToSquare(robotX, robotY, SquareState::UNKNOWN, &goalX, &goalY, true);
    
    if (mapComplete) {

        if (abs(robotX - (float)MAP_DIMENTIONS/2) < 0.3 && abs(robotY - (float)MAP_DIMENTIONS/2) < 0.3) {
            // We are done with the map and we are back at the startingpoint.
            return true;
        }
        rmap.findWayToSquare(robotX, robotY, SquareState::START, &goalX, &goalY, true);
    }
    return false;
}

void updateMotorSpeed(){
    if (!dataOnPort(STY_port)) return;
	pair<double, double> motors = receiveFromStyr();
    /*
	//tcp_server.Send("motorspeed,motor1," + to_string(motors.first) + ";");
	//tcp_server.Send("motorspeed,motor2," + to_string(motors.second) + ";");
	cout << "sent motordata SKRRR" << endl;
    cout << "Sent motor1: " + to_string(motors.first) << endl;
    cout << "Sent motor2: " + to_string(motors.second) << endl;
    */
}

bool updateAll(float& robotX, float& robotY, 
               int& goalX, int& goalY, RPlidarDriver* lidar,
               bool& mapComplete, int& senMessage, float& robotAngle,
               float& turnAngle, bool& newDirection, RobotMap& rmap)
{
    updateMapFromLidar(lidar, robotX, robotY, robotAngle, rmap, senMessage);
    //cout << "Updated map" << endl;

    if (abs(robotX - round(robotX)) < 0.2 && abs(robotY - round(robotY)) < 0.2  && goalX != -1 && goalY != -1) {
        newDirection = false;
    } else {
        newDirection = true;
        if (updatePathfinding(goalX, goalY, mapComplete, robotY, rmap, robotX)) return true;

        updateAngle(goalX, goalY, robotX, robotY, robotAngle, turnAngle);
    }

    sendUpdateToPC(goalX, goalY, robotX, robotY, robotAngle, rmap);
    //cout << "Sent update to pc" << endl;
    
    updateMotorSpeed();
    return false;
}

void updateMode(bool& manualMode, bool& autoMode)
{
    auto response = receiveFromSen();
    Comfromsen senResponse = response.first;
    if (senResponse == Comfromsen::MANUAL) {
        manualMode = true;
        autoMode = false;
        tcp_server.Send("mode,manual;");
        cout << "sent mode,manual;" << endl;
    }
    else if (senResponse == Comfromsen::AUTO) {
        manualMode = false;
        tcp_server.Send("mode,auto;");
        cout << "sent mode,auto;" << endl;
        tcp_server.Send("status,Waiting to start;");
        cout << "sent status,Waiting to start;" << endl;
    }
    else if (senResponse == Comfromsen::ENABLE_MAPPING && !manualMode) {
        autoMode = true;
        tcp_server.Send("mode,Exploring;");
        cout << "sent mode,Exploring;" << endl;
    }
    else if (senResponse == Comfromsen::GYRO_ANGLE) {
        tcp_server.Send("gyro," + to_string(response.second) + ";");
        cout << "sent gyro," + to_string(response.second) + ";" << endl;
    }
}

int main()
{
    // Connects to lidar on port 2.
    RPlidarDriver* lidar;
    lidar = initLidar(2);
    cout << "Lidar has init" << endl;


    int senMessage;
    initKomHub(); //Init the KomHub on RaspberryPi to transmitsignal to Styrenhet.
    sendToSen(REQUEST_MODE);
    
    // Waits for a client to successfully connect
    int response;
    do {
        response = tcp_server.StartServer();
    } while (response != 0);
    cout << "Connected to client" << endl;
    tcp_server.Send("status,Connected;");


    bool manualMode = false;
    bool autoMode = false;
    //tcp_server.Send("status,Waiting to start;");
    while(true) {
        //cout << "In main while" << endl;
        // Checks if PC has sent a start command.
        /*
        vector<vector<string>*>* reply = tcp_server.Recive();
        if (reply->size() != 0) {
            for(vector<string>* command : *reply) {
                if ((*command)[0] == "start") {
                    autoMode = true;
                    tcp_server.Send("status,Start recieved;");
                } else if ((*command)[0] == "mode") {
                    if ((*command)[1] == "auto"){
                        manualMode = false;
                        tcp_server.Send("mode,waiting for start;");
                    } else if ((*command)[1] == "manual"){
                        manualMode = true;
                        tcp_server.Send("mode,manual;");
                    }
                }
                delete command;
            }
        }
        delete reply;
        */

        updateMode(manualMode, autoMode);

        float robotX, robotY, robotAngle;
        int goalX = -1; 
        int goalY = -1;
        bool mapComplete = false;
        RobotMap rmap;
        if (autoMode || manualMode)
        {
            startLidar(lidar);
            cout << "Lidar has started" << endl;
            
            sendToSen(COMPARE_ANGLES, 90);

            robotX = (float)MAP_DIMENTIONS/2;
            robotY = (float)MAP_DIMENTIONS/2;
            robotAngle = 90;
            rmap = RobotMap();
            string mapChanges = rmap.getMapChanges();
            if (mapChanges != "") {
                if (!tcp_server.Send(mapChanges))
                    goto STOP;      // If send fails, stop the robot and lidar
                rmap.clearMapChanges();
            }
            cout << "Map is sent" << endl;
        }

        // RUNS MANUAL MODE
        if (manualMode)
            cout << "Manual mode engaged!" << endl;
        while (manualMode) {
            //usleep(250000);
            auto reply = tcp_server.Recive();
            //cout << "Recieved" << endl;
            if (reply->size() != 0)
            {
                cout << "Recieved data: "; 
				for(auto command : *reply)
				{
					for(auto arg : *command)
					{
						cout << arg << ' ';
					}
				}
				cout << endl;

                for(auto command : *reply)
                {
                    cout << "Sending to styr" << endl;
                    sendToStyr(command);
                    //cout << "Sent to styr" << endl;
                    delete command;
                }
                cout << endl;
            }
            delete reply;

            float turnAngle;
            bool newDirection;
            
            //cout << "Updating all" << endl;
            if (updateAll(robotX, robotY, goalX, goalY, lidar, mapComplete, senMessage, robotAngle, turnAngle, newDirection, rmap)) cout << "We've won!" << endl;
            //cout << "Updated all" << endl;
            if (!tcp_server.gethasConnection()) goto STOP;      // If send has failed, stop the robot and lidar
            
            //cout << "Updated mode" << endl;
            updateMode(manualMode, autoMode);
            //cout << "Updated mode" << endl;
        }

        // RUNS AUTO MODE
        while (autoMode) {
            if (!tcp_server.gethasConnection()) goto STOP;      // If send has failed, stop the robot and lidar
            float turnAngle;
            bool newDirection;
            
            if (updateAll(robotX, robotY, goalX, goalY, lidar, mapComplete, senMessage, robotAngle, turnAngle, newDirection, rmap))
                break;
            
            vector<vector<string>*>* reply = tcp_server.Recive();
            if (reply->size() != 0) {
                for(vector<string>* command : *reply) {
                    for(string arg : *command) {
                        if (arg == "Stay")
                            tcp_server.Send("status,Stay recieved;");
                    }
                    delete command;
                }
            }
            delete reply;
            
            if (newDirection) {
                vector<string> command;
                string angleToSend = to_string(round(fmod(turnAngle,360)));
                cout << angleToSend << endl;
                command.push_back("MoveAuto");
                command.push_back(angleToSend);
                sendToStyr(&command);
            }

            updateMode(manualMode, autoMode);
        }
        if (mapComplete) {
            cout << "We are done!" << endl;
            tcp_server.Send("mode,Exploration complete!;");
            if (!tcp_server.Send("mode,Done!;"))
                goto STOP;      // If send fails, stop the robot and lidar

            vector<string> command;
            command.push_back("Stay");
            sendToStyr(&command);

            stopLidar(lidar);

            mapComplete = false;
            autoMode = false;
            //manualMode = false;
        }
    }
    
    STOP:
    cout << "\033[31mStopped\033[0m" << endl;
    vector<string> command;
    command.push_back("Stay");
    sendToStyr(&command);

    stopLidar(lidar);
    disconnectLidar(lidar);
    cout << "\033[31mStopping completed\033[0m" << endl;
    return 0;
}
