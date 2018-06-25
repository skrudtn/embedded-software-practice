#include <linux/types.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/cdev.h>
#include <linux/wait.h>

/* in nanoseconds */
#define SLEEP_HIGH      10000000UL

#define TCS_S0_PIN          18
#define TCS_S1_PIN          23
#define TCS_S2_PIN          24
#define TCS_S3_PIN          25

#define TCS_ENABLE_PIN      8
#define TCS_OUT_PIN         7

#define DEV_NAME "tcs_dev"

enum STATE {
	READ_WHITE_HEAD = 1,
	READ_RED,
	READ_GREEN,
	READ_BLUE,
	READ_WHITE_TAIL,
	READ_DONE,
};

struct tcs3200_measurement {
	uint32_t white_head;
	uint32_t red;
	uint32_t green;
	uint32_t blue;
	uint32_t white_tail;
};

struct tcs_dev {
	struct cdev cdev;
	dev_t devid;
	int enabled;
	unsigned long dwell;
	struct hrtimer timer;
	wait_queue_head_t waitq;
	int irq;
	enum STATE state;
	struct tcs3200_measurement measurement;
};

enum tcs_output_frequency {
	PWR_DOWN = 1,
	LOW,
	MED,
	HIGH,
};

enum tcs_color {
	RED = 1,
	GREEN,
	BLUE,
	WHITE,
};

/* hw control */
void tcs_enable(struct tcs_dev *);
void tcs_disable(struct tcs_dev *);
int tcs_setup_color(enum tcs_color);
int tcs_setup_frequency(struct tcs_dev *, enum tcs_output_frequency);
int tcs_control_init(struct tcs_dev *);
void tcs_control_exit(void);
/* frequency counter stuff */
int tcs_counter_init(struct tcs_dev *);
void tcs_counter_exit(struct tcs_dev *);
int tcs_start_measurement(struct tcs_dev *);
int tcs_stop_measurement(struct tcs_dev *);

