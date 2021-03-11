/*
 * PWM.c
 *
 * Created: 2020-11-09 16:11:25
 *  Author: Ludvig Widen
 */ 

/*

https://www.circuitstoday.com/dc-motor-speed-control-using-pwm-avr
https://www.electronicwings.com/avr-atmega/dc-motor-interfacing-with-atmega-1632
https://www.avrfreaks.net/forum/tut-c-pwm-complete-idiots
*/

/*

*/

#include <avr/io.h>

#define baseSpeed 80
#define turnSpeed 160
#define rotateSpeed 170
#define noSpeed 255

volatile uint8_t speed = 10;
volatile enum {LEFT, LEFTFORWARD, FORWARD, RIGHTFORWARD, RIGHT, BACK, STOP, CALIBRATE} commando;


void IOinit(void){
	
	DDRB |= (1<<PORTB0) |(1<<PORTB1) |(1<<PORTB2) |(1<<PORTB3) | (1<<PORTB4); // ENABLE OUTPUT PINS
	OCR0A = 255; // Start without motors rotating
	OCR0B = 255;
	
	TCCR0A = (1<<COM0A1) | (1<<COM0A0) | (1<<COM0B1) | (1<<COM0B0) | (1<<WGM00); //High on match when upcounting, Low on match when downcounting, phase correct
	TCCR0B = (1<<CS00); // prescaler /1024 ,
	TCNT0 = 0; // init counter

}	


setMotors(uint8_t rotDir, uint8_t rightSpeed, uint8_t leftSpeed){
	PORTB = rotDir;
	OCR0A = rightSpeed;
	OCR0B = leftSpeed;
} 


void calibrate(void){
	
}



int main(void)
{
	
	IOinit();
	
	commando = FORWARD;
    while(1)
    {
		
		switch(commando){
			case LEFT:
				setMotors(2, rotateSpeed, rotateSpeed);
				break;
			case LEFTFORWARD:
				setMotors(3, turnSpeed, baseSpeed);
				break;
			case FORWARD:
				setMotors(3, baseSpeed, baseSpeed);
				break;
			case RIGHTFORWARD:
				setMotors(3, baseSpeed, turnSpeed);
				break;
			case RIGHT:
				setMotors(1, rotateSpeed, rotateSpeed);
				break;
			case STOP:
				setMotors(0, noSpeed, noSpeed);
				break;
			case BACK:
				setMotors(0, baseSpeed, baseSpeed);
				break;
			default:
				setMotors(0, baseSpeed, baseSpeed);
		}
    }
}