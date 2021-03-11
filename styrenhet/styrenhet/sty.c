/*
 * GccApplication2.c
 *
 * Created: 2020-11-05 13:30:51
 * Author : Hannes Westander, Emil Wiman
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "sty.h"
#include "usart.h"
#include "PWM.h"
#include <stdlib.h>

#define F_CPU 1000000UL  // 1 MHz
//#define F_CPU 14.7456E6
#include <util/delay.h>



//**************************************************************************************************************************
// MAIN
//**************************************************************************************************************************

int main(void)
{
    /* Main loop for sty-module */
		init_sty();
		USART_INIT();
		PWMinit();
		uint8_t current_command, number_of_args;
		sei();
		char received_argument = 0;
		uint16_t high_bits = 0; 
		uint16_t req_degree = 0;
		uint16_t low_bits = 0;

		
    while (1){
		
		if(ready_command){ 
			/* If we have received a command from kom-module, unpack command and nr of arguments. */
			cli(); // disable interrupts.
			current_command = val_kom[0] & 0xF0;
			current_command = (current_command >> 4);	
			number_of_args = val_kom[0] & 0x0F;
			 
			//Fjärrstyrt läge
			if(number_of_args == 0){
				/*No arguments */
				received_argument = 0;
				handleCommands(current_command);
			//Autonomt läge	
			} else{
				/*Arguments we need to unpack */
				received_argument = 1;									
				high_bits = val_kom[1];
				 low_bits = val_kom[2];	
				req_degree = (high_bits << 8) | low_bits;
			}
			ready_command = 0;
			sei(); // enable interrupts
		}
		uint8_t speed;
		if(received_argument){
			/* Identify command it is and execute it*/
			switch(current_command){
				case AUTO:
					if(req_degree <= 359){
						autonom_turn(req_degree);
						autonom_forward(req_degree);
					}
					break;
				case SET_SPEED:
					speed = val_kom[1];
					set_speed(speed);
					break;
				default:				
				handleCommands(STOP);		//In case something goes wrong, stop the robot.			
			}
		}
    }
}



//**************************************************************************************************************************
// Autonom Steering
//**************************************************************************************************************************


uint16_t get_current_angle(){
	/*Asks sen-module for the current (absolute) angle and returns it */
	USART1_Send(0x00);
// 	while(!ready_data){
// 		/*Wait for the data to be receieved*/
// 	}
	_delay_ms(5);
	ready_data = 0;
	
	uint16_t high_bits = val_sen[1];
	uint16_t low_bits = val_sen[2];
	uint16_t curr_degree = high_bits;
	curr_degree = (high_bits << 8) | low_bits;
	//curr_degree = curr_degree + low_bits;
	//curr_degree = curr_degree
	return curr_degree;
}

int get_difference(uint16_t requested){
	/* Returns the difference between the current angle(SEN) and the requested angle(STY), negative diff means we turn left and for positive we turn right */
	int current = get_current_angle();
	int diff = current - requested;
	
	
	/* if we try to turn more than 180 degrees in either direction we turn in the other direction*/
	if (diff > 180){ 
		return diff - 360;
	}
	else if(diff < -180){
		return diff + 360;
	}
	return diff;
}

void autonom_forward(uint16_t requested_angle){
	/* Orders the robot to go forward and tries to stabilize if the robot gets too far off from the requested angle */
	int diff;
	while(!ready_command){
		handleCommands(FORWARD);
		diff = get_difference(requested_angle);
		while((abs(diff) > offset) && !ready_command){
			/*If the difference is outside our accepted range we compensate by turning in the appropriate direction */
			if(diff < 0){
				handleCommands(LEFTFORWARD);
			}
			else{
				handleCommands(RIGHTFORWARD);
			}
			diff = get_difference(requested_angle);
			_delay_ms(10);
		}
	}
}

void autonom_turn(uint16_t requested_angle){
	int diff = get_difference(requested_angle);
	
	while((abs(diff) > offset) && !ready_command){
		if(diff < 0){
			handleCommands(LEFT);
 		}
 		else{
 			handleCommands(RIGHT);
 		}
		
		diff = get_difference(requested_angle);
		//_delay_ms(20);
	}
}


//**************************************************************************************************************************
// Initialization
//**************************************************************************************************************************

void init_sty(void){
/* define directions for port pins*/
DDRB =(1<<DDB3) | (1<<DDB2) | (1<<DDB1) | (1<<DDB0); // utsignaler till roboten 
DDRD =(1<<DDD5) | (1<<DDD3) | (1<<DDD1);
DDRC =(1<<DDC4); //utsignal till JTAG
}

