#ifndef USART_HEADER
#define USART_HEADER


volatile uint8_t val_sen [3];
volatile uint8_t val_kom [3];
volatile char ready_command;
volatile char ready_data;

void USART_INIT ();
void USART0_Send(uint8_t data);
void USART1_Send(uint8_t data);
uint8_t USART0_ReceiveByte();
uint8_t USART1_ReceiveByte();

#endif