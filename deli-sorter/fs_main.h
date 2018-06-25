#define MOTOR_PIN1	6
#define MOTOR_PIN2	13
#define MOTOR_PIN3	19
#define MOTOR_PIN4	26

#define SLEEP_SPEED	1000UL

int steps[8][4]={
	{1,0,0,0},
	{1,1,0,0},
	{0,1,0,0},
	{0,1,1,0},
	{0,0,1,0},
	{0,0,1,1},
	{0,0,0,1},
	{1,0,0,1},
};

static void run_conveyor_motor(void);


