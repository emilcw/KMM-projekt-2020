/*
 * usart.h
 *
 * Created: 2020-12-03 08:25:00
 *  Author: Ludvig Widen, Henrik Träff
 */ 
#ifndef USART_HEADER
#define USART_HEADER


/* Array for storing arguments sent to SEN from STY through UART */
volatile uint8_t usart0_packets[4];
/* Array for storing arguments sent to SEN from KOM through UART */
volatile uint8_t usart1_packets[4];
/* Initialize UART-ports */
void USART_INIT (void); 
/* Send through UART-port to STY */
void USART0_Send(uint8_t data);
/* Send through UART-port to KOM */
void USART1_Send(uint8_t data);

#endif