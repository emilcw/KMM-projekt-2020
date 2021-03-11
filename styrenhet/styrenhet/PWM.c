/*
 * PWM.c
 *
 * Created: 2020-11-09 16:11:25
 *  Author: Hannes Westander Ludvig Widen
 */ 


#include <avr/io.h>
#include "PWM.h"
#include "usart.h"


//**************************************************************************************************************************
// Initialization
//**************************************************************************************************************************

/*


*/
void PWMinit(void){
	
	
	baseSpeed = 130;
	rotateSpeed = 125;

	
	DDRB |= (1<<PORTB0) |(1<<PORTB1) |(1<<PORTB2) |(1<<PORTB3) | (1<<PORTB4); // ENABLE OUTPUT PINS
	OCR0A = 255; // Start without motors rotating
	OCR0B = 255;
	
	TCCR0A = (1<<COM0A1) | (1<<COM0A0) | (1<<COM0B1) | (1<<COM0B0) | (1<<WGM00); //High on match when upcounting, Low on match when downcounting, phase correct
	TCCR0B = (1<<CS00); // prescaler /1024 ,
	TCNT0 = 0; // init counter

}	


//**************************************************************************************************************************
// Handler
//**************************************************************************************************************************


/*
Sets direction on wheels, right and left wheel speed.
*/
void setMotors(uint8_t rotDir, uint8_t rightSpeed, uint8_t leftSpeed){
	PORTB = rotDir;
	OCR0A = rightSpeed;
	OCR0B = leftSpeed;
} 
/* 
Changes the different speed variables.
*/
void set_speed(uint8_t speed){
	switch(speed){
		case 1:
			baseSpeed = 150;
			rotateSpeed = 125;
			break;
		case 2:
			baseSpeed = 120;
			rotateSpeed = 125;
			break;
		case 3: 
			baseSpeed = 100;
			rotateSpeed = 100;
			break;
		case 4:
			baseSpeed = 70;
			rotateSpeed = 40;
			break;
		default:
			break;	
	}
}



/*Executes motor commands */
void handleCommands(uint8_t commandoID)
{	
		switch(commandoID){
			case LEFT:
				setMotors(2, rotateSpeed, rotateSpeed);
				sendSpeed(1,0,rotateSpeed,rotateSpeed);
				break;
			case LEFTFORWARD:
				setMotors(3, turnSpeed2, turnSpeed1);
				sendSpeed(0,0,turnSpeed2,turnSpeed1);
				break;
			case FORWARD:
				setMotors(3, baseSpeed, baseSpeed);
				sendSpeed(0,0,baseSpeed,baseSpeed);
				break;
			case RIGHTFORWARD:
				setMotors(3, turnSpeed1, turnSpeed2);
				sendSpeed(0,0,turnSpeed1, turnSpeed2);
				break;
			case RIGHT:
				setMotors(1, rotateSpeed, rotateSpeed);
				sendSpeed(0,1,rotateSpeed,rotateSpeed);
				break;
			case STOP:
				setMotors(0, noSpeed, noSpeed);
				sendSpeed(0,0,noSpeed,noSpeed);
				break;
			case BACK:
				setMotors(0, backSpeed, backSpeed);
				sendSpeed(1,1,backSpeed,backSpeed);
				break;
			default:
				setMotors(3, noSpeed, noSpeed);
				sendSpeed(0,0,noSpeed, noSpeed);
	  }
}

void sendSpeed(int dirLeft, int dirRight, uint8_t leftSpeed, uint8_t rightSpeed){
	uint8_t message = 0x92;
	uint8_t speed1;
	uint8_t speed2;
	speed1 = (1-(leftSpeed/255) )* 3.6 * 10;
	if(dirLeft){
		speed1 = speed1 || 0x80 ;
	}
	if(dirRight){
		speed2 = speed2 || 0x80 ;
	}
	speed2 = (1-(rightSpeed/255) )* 3.6 * 10;
	USART0_Send(message);
	USART0_Send(speed1);
	USART0_Send(speed2);
	
}