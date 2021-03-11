/*
 * usart.c
 *
 * Created: 2020-12-03 08:25:00
 *  Author: Ludvig Widen, Henrik Träff
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart.h"

// #define USART_BAUDRATE 38400
#define BAUD 9600
#define BAUD_PRESCALE (((16000000 / (BAUD * 16UL))) - 1)


//**************************************************************************************************************************
// UART
//**************************************************************************************************************************
//14600
void USART_INIT(){
	
	//USART0
	/*set baud rate*/
	UBRR0H = (unsigned char)(BAUD_PRESCALE>>8);
	UBRR0L = (unsigned char)BAUD_PRESCALE;
	DDRD |= (1<<PORTD1);
	/*Enable receiver and transmitter and receive complete interrupt */
	UCSR0B = ((1<<TXEN0)|(1<<RXEN0) | (1<<RXCIE0));
	/* Set frame format: 8data, 1stop bit */
	UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);
	
	//USART1
	/* set baud rate */
	UBRR1H = (unsigned char)(BAUD_PRESCALE>>8);
	UBRR1L = (unsigned char)BAUD_PRESCALE;
	DDRD |= (1<<PORTD3);
	/*Enable receiver and transmitter and receive complete interrupt */
	UCSR1B = ((1<<TXEN1)|(1<<RXEN1)|(1<< RXCIE1));
	/* Set frame format: 8data, 1stop bit */
	UCSR1C = (1<<UCSZ10)|(1<<UCSZ11);
	
}

void USART0_Send(uint8_t data){
	
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	
	/* Put data into buffer, sends the data */
	UDR0 = data;

  }
void USART1_Send(uint8_t data) {
	/* Wait for empty transmit buffer */
	while(!(UCSR1A & (1<<UDRE1)));
	/* Put data into buffer, send the data */
	UDR1 = data;
}