	
/*
 * gyro.c
 *
 * Created: 2020-12-03 12:58:39
 *  Author: Ludvig Widén, Henrik Träff
 */ 

// **************************************************************************************************************************************
// Includes
// **************************************************************************************************************************************
#include "sensor_enhet.h"

// **************************************************************************************************************************************
// Variables and definitions
// **************************************************************************************************************************************
/* Store value returned by ad-conversion */
volatile int gyro_value;
/* store the angle related to start position */
volatile int degrees_rotated = 9000;
/* store the finalized angle related to start position */
volatile int correct_degrees = 90;

#define div100  40.96/4096
#define OFFSET 2


// **************************************************************************************************************************************
// Functions
// **************************************************************************************************************************************



void update_gyro(){

	ADC_gyro = ADC_read(1); //Sample Output 0.5-4.5 Volt
	ADC_gyro = ADC_gyro >> 1; //exclude noise
	gyro_value = ((ADC_gyro-zero_rate_output)*conversion); //angle/time

	/*Solution to fix fluctuating pins*/
	if((abs(ADC_gyro-zero_rate_output) > 2)){

	degrees_rotated += gyro_value*div100; //Angle relative to start-position. Unit = degree/100
	
	/* degrees_rotated is multiplied by 100, an int is 2^15=~32k and we want to count to 36k. To avoid overflow 
	we calculate between -9k to +27k instead of 0 to 36k */ 
	
	if(degrees_rotated > 27000) {
		degrees_rotated -= 36000;
	} else if(degrees_rotated < -9000) {
		degrees_rotated += 36000;
	}
	/* Degrees correctly scaled between 0 to 360 */
	correct_degrees = degrees_rotated*div100; //correctly scaled angle.
	
	
	/* Bounds */ 
	if(correct_degrees < 0){
		correct_degrees += 360;
	}
	}
}


void calibrate_gyro(int angle) {
	if(abs(get_current_angle() - angle) > 100 ) {
		return;
	} 
	
	
	if((get_current_angle() - OFFSET > angle) || (get_current_angle() + OFFSET < angle)) { // if the difference is more than 5 degrees then we 
		set_start_current_angle(angle);
	}
}

void clear_degrees_rotated() {
	degrees_rotated = 9000;
	correct_degrees = 90;
}


int get_current_angle() {
	return correct_degrees;
}

void set_start_current_angle(int angle) {
	correct_degrees = angle;
	
	if(angle > 270){
		angle -= 360;
	} 
	degrees_rotated = angle*100;
}
