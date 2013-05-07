/*
 * main.c
 */

#include <inttypes.h>
#include "service/logger/logger.h"
#include "hal/generic/timer/gptimer.h"
#include "hal/generic/mpu_subsystem/intcps.h"
#include "kernel/process.h"
#include "kernel/process_manager.h"


#pragma SWI_ALIAS(make_swi, 47);
void make_swi(unsigned int foo, char* bar);

#pragma INTERRUPT(udef_handler, UDEF);
interrupt void udef_handler() {
	int i = 0;
	i += 2;	logger_error("KERNEL PANIC: udef handler.");
}

#pragma INTERRUPT(swi_handler,SWI);
interrupt void swi_handler(unsigned int foo, char* bar) {
	/*
	 * Save return address (R14) on the stack (later this should be saved in the PCB).
	 * As the arguments (R0 = foo, R1 = bar) have to be the top most values of the stack,
	 * they are also saved (again) on the stack.
	 * The stack looks afterwards like:
	 * SP -->	R0
	 * 			R1
	 * 			R14
	 * 			R0 (added from compiler, don't know how to avoid this stack push)
	 * 			R1 (added from compiler, don't know how to avoid this stack push)
	 */
	asm("\t STMFD R13!, {R0-R1, R14}");

	//logger_debug("Hi! This is the SWI-Handler. You have told me: %u: %s", foo, bar);
	logger_logmode();

	/* Working for setting mode after interrupt:
	 * asm("\t MRS R12, SPSR");
	 asm("\t ORR R12, R12, #0x1F");
	 asm("\t MSR SPSR, R12");*/

	//return 4711;
	/*
	 * Remove the self pushed arguments from the stack.
	 * As one register has 32 bit (4 byte), the stack pointer (R13) has to be changed by 8 byte
	 * (2 arguments * 4 byte).
	 * The stack looks afterwards like:
	 * 			R0
	 * 			R1
	 * 	SP -->	R14
	 * 			R0 (added from compiler, don't know how to avoid this stack push)
	 * 			R1 (added from compiler, don't know how to avoid this stack push)
	 */
	asm("\t ADD R13, R13, #8");

	/*
	 * Loads the next value from the stack, which is the return address saved at the beginning of this method
	 * (later this value should come from PCB or something similar). The value will be loaded into the R14
	 * (which usually holds the return address). The code generated by the compiler includes then the
	 * command MOVS	PC, R14, which is the return out of the interrupt;
	 * The stack looks afterwards like:
	 * 			R0
	 * 			R1
	 * 			R14
	 * 	SP -->	R0 (added from compiler, don't know how to avoid this stack push)
	 * 			R1 (added from compiler, don't know how to avoid this stack push)
	 */
	asm("\t LDMFD R13!, {R14}");
}

#pragma INTERRUPT(pabt_handler, PABT);
interrupt void pabt_handler() {
	int i = 0;
	i += 2;
	logger_error("KERNLE PANIC: Prefetch abort.");
}

#pragma INTERRUPT(dabt_handler, DABT);
interrupt void dabt_handler() {
	int i = 0;
	i += 2;
	logger_error("KERNEL PANIC: data abort");
}

ProcessManager_t processManager;
ProcessId_t currentProcessId;
gptimer_t main_timer;
gptimer_t pwm_timer1;
gptimer_t pwm_timer2;
gptimer_t pwm_timer3;

#define LED0_PIN			(1 << 21)
#define LED1_PIN			(1 << 22)
#define GPIO5_OUT			(unsigned int*) 0x4905603C

int led0(void) {
	int i;
	logger_debug("Led 0");
	while (1) {
		for(i = 0; i < 450000; i++) ;
		*(GPIO5_OUT) ^= LED0_PIN;
	}
}

int led1(void) {
	int i;
	logger_debug("Led 1");
	while (1) {
		for(i = 0; i < 900000; i++);
		*(GPIO5_OUT) ^= LED1_PIN;
	}
}

int idle_task(void) {
	while (1) ; /* TODO: look manual for HALT command or similar to reduce power consumption */
}

void turnoff_rgb(void) {
	/* set mode to 4 (GPIO) see p. ~787 of omap35x.pdf */
	unsigned int* CONTROL_PADCONF_UART2_CTS = (unsigned int*)0x48002174; /* GPIO144 15:0 GPIO145 16:32 */
	unsigned int* CONTROL_PADCONF_UART2_TX = (unsigned int*)0x48002178; /* GPIO146 15:0 */
	*(CONTROL_PADCONF_UART2_CTS) = (4 << 16) | 4;
	*(CONTROL_PADCONF_UART2_TX) &= ~7;
	*(CONTROL_PADCONF_UART2_TX) |= 4;

	/* turn off rgb led on dmx interface
	 *  GPIO 144, 146, 145 --> GPIO 5 */
	unsigned int* GPIO5_OE = (unsigned int*)0x49056034;
	unsigned int* GPIO5_DATAOUT = (unsigned int*)0x4905603C;
	unsigned int rgb = (1 << (144 % 32)) | (1 << (146 % 32)) | (1 << (145 % 32));
	*(GPIO5_OE) &= ~rgb;
	*(GPIO5_DATAOUT) &= ~rgb;
}

void do_pwm(void);

void main(void) {
	logger_init();
	logger_debug("\r\n\r\nSystem init...");
	//make_swi(0x4712, "hugo");
	logger_logmode();

	/* init led stuff */
	turnoff_rgb();
	#define GPIO5_DIR  			(unsigned int*) 0x49056094
	*(GPIO5_DIR) |= LED0_PIN | LED1_PIN;

	process_manager_init(&processManager);

	Process_t process1;
	process1.func = &led0;
	currentProcessId = process_manager_add_process(&processManager, &process1);

	Process_t process2;
	process2.func = &led1;
	process_manager_add_process(&processManager, &process2);

	/* idle task */
	Process_t idle_process;
	idle_process.func = &idle_task;
	process_manager_add_process(&processManager, &idle_process);

	//get schedule timer
	gptimer_get_schedule_timer(&main_timer);
	//activate the timer in intcps module
	intcps_activate_gptimer(&main_timer);

	gptimer_config_t conf = gptimer_get_default_timer_init_config();
	gptimer_init(&main_timer, &conf);
	gptimer_start(&main_timer);

	//do_pwm();

	processManager.currentProcessId = idle_process.pid;
	idle_task();
}
void do_pwm() {
	/* start PWM */
	gptimer_pwm_setup();
	gptimer_get_pwm_timer(1, &pwm_timer1);
	gptimer_get_pwm_timer(2, &pwm_timer2);
	gptimer_get_pwm_timer(3, &pwm_timer3);

	gptimer_pwm_clear(&pwm_timer1);
	gptimer_pwm_clear(&pwm_timer2);
	gptimer_pwm_clear(&pwm_timer3);

	gptimer_pwm_config_t pwm_config;
//	pwm_config.PT = PWM_PT_TOGGLE;
//	pwm_config.SCPWM = PWM_SCPWM_DEFAULT_HIGH;
//	pwm_config.TRG = PWM_TRG_OVERFLOW_AND_MATCH;
	pwm_config.high_percentage = 50;
	pwm_config.timer_config->ticks_in_millis = 1;

	gptimer_pwm_init(&pwm_timer1, &pwm_config);
	gptimer_pwm_init(&pwm_timer2, &pwm_config);
	gptimer_pwm_init(&pwm_timer3, &pwm_config);

	gptimer_pwm_clear(&pwm_timer1);
	gptimer_pwm_clear(&pwm_timer2);
	gptimer_pwm_clear(&pwm_timer3);

}

