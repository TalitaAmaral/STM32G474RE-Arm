#include "main.h"
#include <cstdint>
#include "miros.h"


//teste para prod cons
rtos::OSThread produtor_thread;
rtos::OSThread consumidor_thread;

uint32_t stack_produtor[256];
uint32_t stack_consumidor[256];

rtos::OSComunication comunicacao_teste;
void main_produtor(void);
void main_consumidor(void);
// fim teste



uint32_t conta0=0, conta1=0, conta2=0;

uint32_t stack_blinky1[40];
rtos::OSThread blinky1;
void main_blinky1() {
    while (1) {
    	conta0++;
    	rtos::OS_delay(rtos::TICKS_PER_SEC * 3U / 4U);
    }
}

uint32_t stack_blinky2[40];
rtos::OSThread blinky2;
void main_blinky2() {
    while (1) {
    	conta1++;
    	rtos::OS_delay(rtos::TICKS_PER_SEC / 3U);
    }
}

uint32_t stack_blinky3[40];
rtos::OSThread blinky3;
void main_blinky3() {
    while (1) {
    	conta2++;
    	rtos::OS_delay(rtos::TICKS_PER_SEC * 3U / 5U);
    }
}

uint32_t stack_idleThread[40];


int main(void){

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


	  // teste prod cons
	  rtos::OSThread_start(&produtor_thread,
	                 &main_produtor,
	                 stack_produtor, sizeof(stack_produtor));

	        /* start consumidor thread */
	  rtos::OSThread_start(&consumidor_thread,
	                 &main_consumidor,
	                 stack_consumidor, sizeof(stack_consumidor));



	  /* transfer control to the RTOS to run the threads */
	  rtos::OS_run();
}


// teste prod cons
void main_produtor(void) {
    int32_t item = 100;
    while(1) {
        item++;
        comunicacao_teste.write(item);
        rtos::OS_delay(500); // Produtor lento
    }
}

void main_consumidor(void) {
    int32_t dado_recebido = 0;
    while(1) {
        dado_recebido = comunicacao_teste.read();

        // Usamos a variável aqui para sumir com aquele warning do compilador
        (void)dado_recebido;

        rtos::OS_delay(50);  // Consumidor rápido
    }
}
// fim teste

