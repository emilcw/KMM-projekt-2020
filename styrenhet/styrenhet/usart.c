
#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart.h"

#define BAUD 9600
#define BAUD_PRESCALE (((16000000 / (BAUD * 16UL))) - 1)

/*Global variables*/
volatile uint8_t kom_counter = 0;
volatile uint8_t sen_counter = 0;
volatile uint8_t numOfArgs = 0;


//**************************************************************************************************************************
// Interrupts
//**************************************************************************************************************************

/*


*/
ISR(USART0_RX_vect){
	//KOMMUNIKATIONSENHET
	uint8_t temp = UDR0;
	
	if (kom_counter == 0)
	{ 
		numOfArgs = temp & 0x0F; //Read number of args
		val_kom[0] = temp; // save first 8bit-package
	}
	else {
		val_kom[kom_counter] = temp; // save n-th package
	}
	if (kom_counter == numOfArgs){
		ready_command = 1;
		kom_counter = 0;
	}
	else {
		kom_counter++;
	}	    
}

ISR(USART1_RX_vect){
	// SENSORENHET
	uint8_t temp = UDR1;
	
	if (sen_counter == 0 || sen_counter == 1 || sen_counter == 2)
	{	
		val_sen[sen_counter] = temp; // save first 8bit-package (Saving command and angle)
	}	
	if (sen_counter == 2){ // both packages received
		ready_data = 1;
		sen_counter = 0;
	}
	else {
		sen_counter ++;
	}
	
}

//**************************************************************************************************************************
// UART
//**************************************************************************************************************************
void USART_INIT(){
	ready_command = 0;
	ready_data = 0;
	/*set baud rate*/
	UBRR0H = (unsigned char)(BAUD_PRESCALE>>8);
	UBRR0L = (unsigned char)BAUD_PRESCALE;
	DDRD = (1<<PORTD1);
	/*Enable receiver and transmitter and receive complete interrupt */
	UCSR0B = ((1<<TXEN0)|(1<<RXEN0) | (1<<RXCIE0));
	/* Set frame format: 8data, 1 stop bit */
	UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);
	
	UBRR1H = (unsigned char)(BAUD_PRESCALE>>8);
	UBRR1L = (unsigned char)BAUD_PRESCALE;
	DDRD |= (1<<PORTD3);
	/*Enable receiver and transmitter and receive complete interrupt */
	UCSR1B = ((1<<TXEN1)|(1<<RXEN1)|(1<< RXCIE1));
	/* Set frame format: 8data, 1stop bit */
	UCSR1C = (1<<UCSZ10)|(1<<UCSZ11);

}

void USART0_Send(uint8_t data){
	//Transmit to KOM
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	/* Put data into buffer, sends the data */
	UDR0 = data;

  }
  
void USART1_Send(uint8_t data){
	//Transmit to SEN
	/* Wait for empty transmit buffer */
	while ( !( UCSR1A & (1<<UDRE1)) );
	/* Put data into buffer, sends the data */
	UDR1 = data;

}

uint8_t USART0_ReceiveByte(){
	/* Receive from Kom*/
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) );
	/* Get and return received data from buffer */
	return UDR0;
}

uint8_t USART1_ReceiveByte(){
	/* Receive from Sen*/ 
	/* Wait for data to be received */
	while ( !(UCSR1A & (1<<RXC1)) );
	/* Get and return received data from buffer */
	return UDR1;
}


void USART_flush(void){
	unsigned char dummy;
	while(UCSR0A & (1<<RXC0)){
		dummy = UDR0;
	}
}
