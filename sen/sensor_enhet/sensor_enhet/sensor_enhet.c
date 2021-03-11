/*
 * sensor_enhet.c
 *
 * Created: 2020-12-03 08:38:39
 *  Author: Ludvig Widen, Henrik Träff
 */ 


// **************************************************************************************************************************************
// Includes
// **************************************************************************************************************************************
#include "sensor_enhet.h"

// **************************************************************************************************************************************
// Variables and definitions
// **************************************************************************************************************************************
/* global flags for ad-conversion interrupt and uart interrupt */
volatile char adc_interrupt_flag, uart_interrupt;
/* Variables for handling commando from UART-ports  */
volatile uint8_t usart0_commando, usart1_commando;
/* Variable for keeping track of number of args/packets to come through UART */
volatile uint8_t num_of_args = 0;
/* Help variable for UART-interrupt-function */
volatile uint8_t counter = 0;
/*Enum definition to keep track of who requested data  */
volatile enum MODULE{STY, KOM} module;
/*Enum definition to keep track of the state */	
volatile enum STATE{AUTO, MANUAL} state = MANUAL;
/*Enum definition to keep track of if we are mapping or not */
volatile enum MAPPING{ENABLED, DISABLED} mapping = DISABLED;
/* Angle from kom, >8 bits because 0-360 */
volatile uint16_t angle_from_kom = 0;
	
// **************************************************************************************************************************************
// I/O
// **************************************************************************************************************************************
void IOinit(){
	DDRB = 0xF3; // PB2 and PB3	 is made input to handle external interrupts rest outputs.
	DDRA = 0xFC;
}


// **************************************************************************************************************************************
// Interrupt Routines
// **************************************************************************************************************************************
ISR(TIMER0_COMPA_vect){
	adc_interrupt_flag = 1;
	TCNT0 = 0;
	TIFR1 |= (1 << OCF0A);
}


/*
Interrupt: MAPPING DISABLED/ENABLED
*/
 ISR(INT2_vect) {
	 if(state == AUTO){
		 USART1_Send(MAPPING_ENABLED);	
	 }	
 }


/*
Interrupt: AUTO/MANUAL
*/
ISR(PCINT1_vect){
	change_state();
}


/* USART SEN - STY */
ISR(USART0_RX_vect){
	/* Receive data, need to understand what has been received  */
	module = STY;
	usart0_commando = UDR0;
	usart0_commando = usart0_commando >> 4;
	uart_interrupt = 1;
}

/* USART SEN - KOM */
ISR(USART1_RX_vect){
	/*Receive data, need to understand what has been received  */

 	uint8_t temp1 = UDR1;
 	module = KOM;
 
 	if (counter == 0)
 	{
 		num_of_args = temp1 & 0x0F; //Read nr of args
 		usart1_packets[0] = temp1; // save first 8bit-package
 	
 	}
 	else {
 		usart1_packets[counter] = temp1; // save n-th package
 	}
 
 	if (counter == num_of_args){
		usart1_commando = usart1_packets[0] & 0xF0;
		usart1_commando = usart1_commando >> 4;
		uart_interrupt = 1;
		counter = 0;
		/* If angle is above 255 we need more than 8bits */
		if(num_of_args == 2){
			//angle_from_kom = ((usart1_packets[1] << 8) | usart1_packets[2]); // 0000 0001 & 1111 1111 = 0000 0001 1111 1111
			int arg1 = usart1_packets[1] * 256;
			angle_from_kom = arg1 + usart1_packets[2];
		}
	}
	else {
		counter ++;
	}
}

// **************************************************************************************************************************************
// ADC
// **************************************************************************************************************************************
void ADC_init(){
	ADMUX = (1<<REFS0); //| (1 << REFS1) // Initialize ADMUX, AREF = AVcc; - Mux selection reg
	ADMUX |= (1<<ADLAR);
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);	//ADC ENABLE / ADC INTERRUPT ENABLE / PRESCALER 128
	zero_rate_output = ADC_read(1);
	zero_rate_output = zero_rate_output >> 1; //exclude noise
}

/* Returns ADC status running/ready */
char ADC_ready(){
	return (ADCSRA & (1<<ADIF)); //ADIF=1 when conversion is done
}

/* Conversion */
uint8_t ADC_read(uint8_t ch){	
	ADMUX &= 0xE0; //clear old channel.
	ADMUX |= ch; //set new channel
	ADCSRA |= (1<<ADSC); // start single conversion
	while(!ADC_ready()); //Wait until the conversion is complete
	return (ADCH); // ADCH
}


// **************************************************************************************************************************************
// Timer
// **************************************************************************************************************************************
void timer0_init(void)
{
	TCNT0 = 0; //reset timer register
	//TIMSK0 = (1<<TOIE0); // enable timer overflow interrupt
	TIMSK0 = (1<<OCIE0A);
	TCCR0B = (1<<CS02) | (1<<CS00);   // increments the timer with a /1024 prescaler
	OCR0A = 155;
	
}


// **************************************************************************************************************************************
// STATES AND MODES
// **************************************************************************************************************************************

/*Extern interrupt on pin PB2
MODE : AUTO / MANUAL */
void extern_interrupt_init() {
	/* Enable int2*/
	EICRA |= (1<<ISC20) | (1<<ISC21);
	EIMSK |= (1<< INT2);
	/* Enable pb3 interrupt */
	PCICR |= (1<<PCIE1); // Pin change interrupt pin 15..8
	PCMSK1 |= (1<<PCINT11); 
}

/* Called when switch is turned*/
void change_state(){
	if(state == AUTO){
		state = MANUAL;
		USART1_Send(MANUAL_STATE);
	} else{
		clear_degrees_rotated();
		state = AUTO;
		USART1_Send(AUTO_STATE);
	}
}

// **************************************************************************************************************************************
// Main
// **************************************************************************************************************************************
int main(void)
{	
	IOinit();
	uint8_t commando;
	timer0_init();
	ADC_init();
	extern_interrupt_init();
	adc_interrupt_flag = 0;
	uart_interrupt = 0;
	USART_INIT();
	sei();
	
	while(1)
	{
		
		
		/* ADC TIMER INTERRUPT */
		if(adc_interrupt_flag){
			adc_interrupt_flag = 0;
			cli();
			update_gyro();	
			sei();
 		}
	 
			
		/* UART INTERRUPT */
		if(uart_interrupt){
			cli();
			if(module == STY){
				commando = usart0_commando;  
			} else{
				commando = usart1_commando;
			}
			/* Send requested data */
			switch(commando)
			{
				case GYRO_DATA_TO_STY:
					USART0_Send(GYRO_DATA); 
					int deg = get_current_angle();
					if(deg > 255){
						USART0_Send(0x01); // > 8 bits 
						USART0_Send(deg);
					} else {
						USART0_Send(0x00); // < 8 bits
						USART0_Send(deg);
					}
					break;
				case GYRO_DATA_TO_KOM:
					USART1_Send(GYRO_DATA);
					int deg1 = get_current_angle();
					if(deg1 > 255){
						USART1_Send(0x01); // > 8 bits
						USART1_Send(deg1);
						} else {
						USART1_Send(0x00); // < 8 bits
						USART1_Send(deg1);
					}		
					break; 
				case RESET_GYRO:
					clear_degrees_rotated();
					break;
				case COMPARE_ANGLES:
					calibrate_gyro(angle_from_kom);
					break;
				case REQUEST_MODE:
					if(state == MANUAL) {
						USART1_Send(MANUAL_STATE);
					}
					else {
						USART1_Send(AUTO_STATE);
								
					}
					break;
				default:
					break;
			}
			uart_interrupt = 0;
			sei();
		}
	}
} 
