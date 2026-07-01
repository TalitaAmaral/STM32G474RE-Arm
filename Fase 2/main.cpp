#include "main.h"
#include <cstdint>
#include "miros.h"
#include "SEGGER_SYSVIEW.h"

uint32_t conta0=0, conta1=0, conta2=0;

uint32_t stack_blinky1[256];
rtos::OSThread blinky1;
void main_blinky1() {
    while (1) {
    	conta0++;
    	rtos::OS_delay(rtos::TICKS_PER_SEC * 3U / 4U);
    }
}

uint32_t stack_blinky2[256];
rtos::OSThread blinky2;
void main_blinky2() {
    while (1) {
    	conta1++;
    	rtos::OS_delay(rtos::TICKS_PER_SEC / 3U);
    }
}

uint32_t stack_blinky3[256];
rtos::OSThread blinky3;
void main_blinky3() {
    while (1) {
    	conta2++;
    	rtos::OS_delay(rtos::TICKS_PER_SEC * 3U / 5U);
    }
}

uint32_t stack_idleThread[256];




int main(void)
{

	  rtos::OS_init(stack_idleThread, sizeof(stack_idleThread));

	  /* start blinky1 thread */
	  rtos::OSThread_start(&blinky1,
	                 &main_blinky1,
	                 stack_blinky1, sizeof(stack_blinky1));

	  /* start blinky2 thread */
	  rtos::OSThread_start(&blinky2,
	                 &main_blinky2,
	                 stack_blinky2, sizeof(stack_blinky2));

	  /* start blinky3 thread */
	  rtos::OSThread_start(&blinky3,
	                 &main_blinky3,
	                 stack_blinky3, sizeof(stack_blinky3));


	  SEGGER_SYSVIEW_Start();

	  /* transfer control to the RTOS to run the threads */
	  rtos::OS_run();
}

/*
extern "C" void SysTick_Handler(void) {
    __disable_irq();
    rtos::OS_tick();
    rtos::OS_sched();
    __enable_irq();
}*/
