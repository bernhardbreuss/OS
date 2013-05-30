/*
 * pwm_test.c
 *
 *  Created on: 10.05.2013
 *      Author: lumannnn
 */


#include "../platform/platform.h"
#include "../hal/generic/timer/gptimer.h"
#include "../hal/generic/pwm/pwm.h"

gptimer_t pwm_timer1;
gptimer_t pwm_timer2;
gptimer_t pwm_timer3;

pwm_config_t pwm_config1;
pwm_config_t pwm_config2;
pwm_config_t pwm_config3;

void waitfor(unsigned int cycles) {
	int i = 0;
	while (++i < cycles) ;
}

void do_pwm() {
	/* start PWM */
	pwm_setup();
	pwm_get_timer(1, &pwm_timer1);
	pwm_get_timer(2, &pwm_timer2);
	pwm_get_timer(3, &pwm_timer3);

	pwm_clear(&pwm_timer1);
	pwm_clear(&pwm_timer2);
	pwm_clear(&pwm_timer3);

	pwm_config1.duty_cycle = pwm_config2.duty_cycle = pwm_config3.duty_cycle = 0;
	pwm_config1.period_time = pwm_config2.period_time = pwm_config3.period_time = 6000;
	pwm_config1.pwm_frequenz = pwm_config2.pwm_frequenz = pwm_config3.pwm_frequenz = 32768;

	int i = 0;
	for (i = 0; i <= 100; ++i) {
		pwm_config1.duty_cycle = i;
		pwm_config(&pwm_timer1, &pwm_config1);
		waitfor(9000);
	}
	for (i = 100; i >= 0; --i) {
		pwm_config1.duty_cycle = i;
		pwm_config(&pwm_timer1, &pwm_config1);
		waitfor(9000);
	}
	pwm_clear(&pwm_timer1);

	for (i = 0; i <= 100; ++i) {
		pwm_config2.duty_cycle = i;
		pwm_config(&pwm_timer2, &pwm_config2);
		waitfor(9000);
	}
	for (i = 100; i >= 0; --i) {
		pwm_config2.duty_cycle = i;
		pwm_config(&pwm_timer2, &pwm_config2);
		waitfor(9000);
	}
	pwm_clear(&pwm_timer2);

	for (i = 0; i <= 100; ++i) {
		pwm_config3.duty_cycle = i;
		pwm_config(&pwm_timer3, &pwm_config3);
		waitfor(9000);
	}
	for (i = 100; i >= 0; --i) {
		pwm_config3.duty_cycle = i;
		pwm_config(&pwm_timer3, &pwm_config3);
		waitfor(9000);
	}
	pwm_clear(&pwm_timer3);

	for (i = 0; i <= 100; ++i) {
		pwm_config1.duty_cycle = i;
		pwm_config2.duty_cycle = i;
		pwm_config3.duty_cycle = i;
		pwm_config(&pwm_timer1, &pwm_config1);
		pwm_config(&pwm_timer2, &pwm_config2);
		pwm_config(&pwm_timer3, &pwm_config3);
		waitfor(9000);
	}
	for (i = 100; i >= 0; --i) {
		pwm_config1.duty_cycle = i;
		pwm_config2.duty_cycle = i;
		pwm_config3.duty_cycle = i;
		pwm_config(&pwm_timer1, &pwm_config1);
		pwm_config(&pwm_timer2, &pwm_config2);
		pwm_config(&pwm_timer3, &pwm_config3);
		waitfor(9000);
	}
	pwm_clear(&pwm_timer1);
	pwm_clear(&pwm_timer2);
	pwm_clear(&pwm_timer3);
}
