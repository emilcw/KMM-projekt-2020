#ifndef KOMHUB_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define KOMHUB_H
#include <wiringSerial.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <bits/stdc++.h>
#include <iostream>


using namespace std;


/*This enum list contains the different commands to our robot.
 * To add a new command, add it here and add in 
 * "getCorrespondingEnum" and "readCommand"* to make it work.*/

enum Commando {FORWARD, FORWARD_LEFT, FORWARD_RIGHT, LEFT, RIGHT, STOP, BACK, CALIBRATE, NO_COMMAND, COMPARE_ANGLES, MOVE_AUTO,
				SET_SPEED, REQUEST_MODE, REQUEST_ANGLE};
enum Comfromsen {AUTO, MANUAL, DISABLE_MAPPING, ENABLE_MAPPING, NOT_ON_PORT, ERROR, GYRO_ANGLE};
/*-------------------------------------------*/
/* Definitions handling commands from SEN*/
#define MANUAL_STATE 0x10 // as defined in SEN
#define AUTO_STATE 0x20 // as defined in SEN
#define MAPPING_ENABLED 0x30//as defined in SEN
#define MAPPING_DISABLED 0x40 // as defined in SEN
#define GYRO_DATA 0x52 // as defined in SEN
/*-------------------------------------------*/

 /*These are global variables to our USB-ports.
  * They are only allowed to be assigned at one place, 
  * namely in initKomHub()*/
extern int STY_port; //Styrenhet
extern int usb1_port; //Sensorenhet
int usb1_port_init();
int STY_port_init();
bool dataOnPort(int usb_port);
void send(int port, char message);
void sendToSen(Commando command, int argument = 0);
void sendToStyr(vector<string>* command);
pair<Comfromsen, int> receiveFromSen();
pair<double, double> receiveFromStyr();
void initKomHub();
Commando getCorrespondingEnum(string command);

#endif // KOMHUB_H
