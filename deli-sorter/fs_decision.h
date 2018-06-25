#define MOTOR_PIN1	22
#define MOTOR_PIN2	10
#define MOTOR_PIN3	9
#define MOTOR_PIN4	11

#define DEV_NAME "motor2_dev"

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

