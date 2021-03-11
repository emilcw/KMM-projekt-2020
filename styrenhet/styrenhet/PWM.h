/*
 * PWM.h
 *
 * Created: 2020-11-13 09:56:07
 *  Author: hanwe828 och Ludvig Widen
 */ 

#ifndef PWM_HEADER
#define PWM_HEADER

#define noSpeed 255

uint8_t baseSpeed;
uint8_t rotateSpeed; 
#define backSpeed 150
#define turnSpeed1 50
#define turnSpeed2 150


#define FORWARD 1
#define LEFTFORWARD 2
#define RIGHTFORWARD 3
#define LEFT 4
#define RIGHT 5
#define STOP 0
#define BACK  6
#define CALIBRATE 7
#define AUTO 8
#define SET_SPEED 9



void PWMinit(void);
void setMotors(uint8_t rotDir, uint8_t rightSpeed, uint8_t leftSpeed);
void calibrate(void);
void handleCommands(uint8_t commandoID);
void set_speed(uint8_t speed);
void sendSpeed(int dirLeft, int dirRight, uint8_t leftSpeed, uint8_t RightSpeed);


#endif