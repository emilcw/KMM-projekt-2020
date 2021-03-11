/*
 * sensor_enhet.c
 *
 * Created: 2020-12-03 08:25:00
 *  Author: Ludvig Widen, Henrik Träff
 */ 
#ifndef SENSOR_ENHET_HEADER
#define SENSOR_ENHET_HEADER

#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/io.h> // avr header file for IO ports
#include <avr/interrupt.h> // avr header file for interrupts
#include "usart.h"
#include "gyro.h"

#define F_CPU 16000000UL
#define conversion 578 //1/(0.00667*127/5)*100 , multiplied by  100 to get rid of floats
#define conversion_11bit 36 //1/(0.00667*2047/5)*100 , multiplied by  100 to get rid of floats

// **************************************************************************************************************************************
// Commands from UART-port
// **************************************************************************************************************************************

#define GYRO_DATA_TO_STY 0
#define REFLEX_DATA_TO_STY 1
#define GYRO_DATA_TO_KOM 2
#define REFLEX_DATA_TO_KOM 3
#define RESET_GYRO 4
#define COMPARE_ANGLES 5
#define REQUEST_MODE 6


// **************************************************************************************************************************************
// Commands to send on UART-port
// **************************************************************************************************************************************
#define MANUAL_STATE 0x10
#define AUTO_STATE 0x20
#define MAPPING_ENABLED 0x30
#define MAPPING_DISABLED 0x40
#define GYRO_DATA 0x52
#define BACKGROUND_DATA 0x60
#define REFLEX_DATA 0x71
// **************************************************************************************************************************************

/*Variable for storing ad-conversion from gyro */                             
volatile uint8_t ADC_gyro;

/*Input and output initialize */
void IOinit();
/* Initialize external interrupt */
void extern_interrupt_init();
/* Initialize ad-conversion */
void ADC_init();
/* Function for checking when ad-conversion is ready */
char ADC_ready();
/* Read ad-conversion data */
uint8_t ADC_read(uint8_t ch);
/* Initialize a 10ms clock */
void timer0_init(void);
/* Start or disable mapping */
void change_mode();
/* Change mode between auto and manual */
void change_state();

#endif
