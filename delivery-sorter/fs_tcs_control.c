#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/ktime.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include "fs_tcs.h"

static enum hrtimer_restart tcs3200_timer(struct hrtimer *timer) {
	struct tcs_dev *tcs;
	enum STATE next;

	tcs = container_of(timer, struct tcs_dev, timer);
	tcs_disable(tcs);
	switch(tcs->state) {
	case READ_WHITE_HEAD:
		tcs_setup_color(RED);
		next = READ_RED;
		break;
	case READ_RED:
		tcs_setup_color(GREEN);
		next = READ_GREEN;
		break;
	case READ_GREEN:
		tcs_setup_color(BLUE);
		next = READ_BLUE;
		break;
	case READ_BLUE:
		tcs_setup_color(WHITE);
		next = READ_WHITE_TAIL;
		break;
	case READ_WHITE_TAIL:
		next = READ_DONE;
		break;
	case READ_DONE:
		tcs_setup_frequency(tcs, PWR_DOWN);
		wake_up_interruptible(&tcs->waitq);
		return HRTIMER_NORESTART;
	}
	tcs->state = next;
	tcs_enable(tcs);
	hrtimer_forward_now(&tcs->timer, ns_to_ktime(tcs->dwell));
	return HRTIMER_RESTART;
}

static irqreturn_t tcs3200_irq(int irq, void *dev_id) {
	struct tcs_dev *tcs = (struct tcs_dev *)dev_id;
	struct tcs3200_measurement *m;

	if(tcs->enabled) {
		m = &tcs->measurement;
		switch(tcs->state) {
		case READ_WHITE_HEAD:
			++m->white_head;
			break;
		case READ_RED:
			++m->red;
			break;
		case READ_GREEN:
			++m->green;
			break;
		case READ_BLUE:
			++m->blue;
			break;
		case READ_WHITE_TAIL:
			++m->white_tail;
			break;
		case READ_DONE:
			break;
		}
	}
	return IRQ_HANDLED;
}

int  tcs_counter_init(struct tcs_dev *tcs) {

	hrtimer_init(&tcs->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	tcs->timer.function = &tcs3200_timer;
	tcs->state = READ_WHITE_HEAD;

	gpio_request_one(TCS_OUT_PIN, GPIOF_IN, "tcs_out");
	tcs->irq = gpio_to_irq(TCS_OUT_PIN);
	if(request_irq(tcs->irq, tcs3200_irq, IRQF_TRIGGER_FALLING,"tcs", tcs)) {
		printk("irq request ERR\n");
		return -1;
	}
	return 0;

}

void tcs_counter_exit(struct tcs_dev *tcs) {
	tcs_disable(tcs);
	hrtimer_cancel(&tcs->timer);
	free_irq(tcs->irq, tcs);
	gpio_free(TCS_OUT_PIN);
}

int tcs_start_measurement(struct tcs_dev *tcs) {
	struct tcs3200_measurement *m = &tcs->measurement;

	memset(m, 0, sizeof(struct tcs3200_measurement));
	tcs_setup_frequency(tcs, HIGH);
	tcs_setup_color(WHITE);
	tcs->state = READ_WHITE_HEAD;
	tcs_enable(tcs);
	hrtimer_start(&tcs->timer, ns_to_ktime(tcs->dwell),HRTIMER_MODE_REL);

	return 0;
}

int tcs_stop_measurement(struct tcs_dev *tcs) {

	tcs_setup_frequency(tcs, PWR_DOWN);
	tcs_disable(tcs);
	tcs->state = READ_WHITE_HEAD;
	return 0;
}

void tcs_enable(struct tcs_dev *tcs) {
	tcs->enabled = 1;
	gpio_set_value(TCS_ENABLE_PIN, 0);
}

void tcs_disable(struct tcs_dev *tcs) {
	tcs->enabled = 0;
	gpio_set_value(TCS_ENABLE_PIN, 1);
}

int tcs_setup_color(enum tcs_color c) {

	switch(c) {
	case RED:
		gpio_set_value(TCS_S2_PIN, 0);
		gpio_set_value(TCS_S3_PIN, 0);
		break;
	case GREEN:
		gpio_set_value(TCS_S2_PIN, 1);
		gpio_set_value(TCS_S3_PIN, 1);
		break;
	case BLUE:
		gpio_set_value(TCS_S2_PIN, 0);
		gpio_set_value(TCS_S3_PIN, 1);
		break;
	case WHITE:
		gpio_set_value(TCS_S2_PIN, 1);
		gpio_set_value(TCS_S3_PIN, 0);
		break;
	default:
		return -1;
	}
	return 0;
}

int tcs_setup_frequency(struct tcs_dev *tcs, enum tcs_output_frequency f) {

	switch(f) {
	case PWR_DOWN:
		gpio_set_value(TCS_S0_PIN, 0);
		gpio_set_value(TCS_S1_PIN, 0);
		break;
	case HIGH:
		gpio_set_value(TCS_S0_PIN, 1);
		gpio_set_value(TCS_S1_PIN, 1);
		break;
	default:
		return -1;
	}
	return 0;
}

int tcs_control_init(struct tcs_dev *tcs) {

	tcs->enabled = 0;
	gpio_request_one(TCS_ENABLE_PIN, GPIOF_INIT_HIGH, "tcs_led");
	gpio_request_one(TCS_S0_PIN, GPIOF_INIT_LOW, "tcs_s0");
	gpio_request_one(TCS_S1_PIN, GPIOF_INIT_LOW, "tcs_s1");
	gpio_request_one(TCS_S2_PIN, GPIOF_INIT_LOW, "tcs_s2");
	gpio_request_one(TCS_S3_PIN, GPIOF_INIT_LOW, "tcs_s3");

	tcs->dwell = SLEEP_HIGH;
	tcs_setup_frequency(tcs, PWR_DOWN);
	tcs_disable(tcs);
	return 0;
}

void __exit tcs_control_exit(void) {

	gpio_free(TCS_S3_PIN);
	gpio_free(TCS_S2_PIN);
	gpio_free(TCS_S1_PIN);
	gpio_free(TCS_S0_PIN);
	gpio_free(TCS_ENABLE_PIN);
}
