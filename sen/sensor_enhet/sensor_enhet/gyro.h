/*
 * gyro.h
 *
 * Created: 2020-12-03 08:50
 *  Author: Ludvig Widén, Henrik Träff
 */ 
#ifndef GYRO_HEADER
#define GYRO_HEADER

/*Global variable for storing the initial angle from gyro */
volatile uint8_t zero_rate_output;
/* Function that reads from adc and updates the current angle */
void update_gyro();
/* Getter for returning angle related to start position */
int get_current_angle();
/* Setter for returning angle related to start position */
void set_start_current_angle(int angle);
/* Clear variable degrees_rotated */
void clear_degrees_rotated();
/* Update gyro-value bases on input from LIDAR */
void calibrate_gyro(int angle);

#endif