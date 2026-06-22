#include "main.h"
#include <cstdint>
#include "miros.h"


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


//teste para prod cons
rtos::OSComunication comunicacao_teste;

uint32_t stack_produtor[256];
rtos::OSThread produtor_thread;
void main_produtor(void) {
    int32_t item = 100;
    while(1) {
        item++;
        comunicacao_teste.write(item);
        rtos::OS_delay(500); // Produtor lento
    }
}

uint32_t stack_consumidor[256];
rtos::OSThread consumidor_thread;
void main_consumidor(void){
    int32_t dado_recebido = 0;
    while(1) {
        dado_recebido = comunicacao_teste.read();
        (void)dado_recebido;
        rtos::OS_delay(50);
    }
}
// fim teste prod cons

// pilhas e thread para a fase 4
uint32_t stack_pid[128];
rtos::OSThread pid_thread;
// fim pilhas e thread para a fase 4

// adição do controle
void main_controle_pid() {
    while (1) {
        // 1. Lê o sensor de distância
        // 2. Calcula o PID
        // 3. Atualiza o PWM do levitador
        
        // Finalizou o trabalho! Cede a CPU até o próximo período (ex: 40 ms)
        rtos::OS_wait_period(); 
    }
}
// fim adição do controle


int main(void){

	  rtos::OS_init(stack_idleThread, sizeof(stack_idleThread));

      // adicionamos periodo=0 no final das threads
	  
	  /* start blinky1 thread */
	  rtos::OSThread_start(&blinky1,
	                 &main_blinky1,
	                 stack_blinky1, sizeof(stack_blinky1), 0);

	  /* start blinky2 thread */
	  rtos::OSThread_start(&blinky2,
	                 &main_blinky2,
	                 stack_blinky2, sizeof(stack_blinky2), 0);

	  /* start blinky3 thread */
	  rtos::OSThread_start(&blinky3,
	                 &main_blinky3,
	                 stack_blinky3, sizeof(stack_blinky3), 0);


	  // teste prod cons
	  rtos::OSThread_start(&produtor_thread,
	                 &main_produtor,
	                 stack_produtor, sizeof(stack_produtor), 0);

	  rtos::OSThread_start(&consumidor_thread,
	                 &main_consumidor,
	                 stack_consumidor, sizeof(stack_consumidor), 0);
      // fim teste prod cons

	  // inicializa o controle PID passando periodo=40ms (tick do EDF)
      rtos::OSThread_start(&pid_thread,
		  			&main_controle_pid,
		  			stack_pid, sizeof(stack_pid), 40);
	  // fim controle PID

	  /* transfer control to the RTOS to run the threads */
	  rtos::OS_run();
}

