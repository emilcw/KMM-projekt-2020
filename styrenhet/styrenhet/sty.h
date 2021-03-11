#ifndef STY_HEADER
#define STY_HEADER

volatile uint8_t driveSpeed;
volatile uint8_t driveAngle;
volatile uint8_t driveDistance;

#define offset 5

void init_sty(void);
void execute_command(uint8_t command);
void demand_sen_info(void);
void handle_sen(void);
void handle_kom(void);
void autonom_turn(uint16_t requested_angle);
void autonom_forward(uint16_t requested_angle);
uint16_t get_current_angle(void);
int get_difference(uint16_t requested_degree);


#endif