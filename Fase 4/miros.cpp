#include <cstdint>
#include "miros.h"
#include "qassert.h"
#include "stm32g4xx.h"

Q_DEFINE_THIS_FILE

namespace rtos {

OSThread * volatile OS_curr; /* pointer to the current thread */
OSThread * volatile OS_next; /* pointer to the next thread to run */

OSThread *OS_thread[32 + 1]; /* array of threads started so far */ //32 bits + 1 bit para a idleThread
uint32_t OS_readySet; /* bitmask of threads that are ready to run */

uint8_t OS_threadNum; /* number of threads started */
uint8_t OS_currIdx; /* current thread index for the circular array */

// inicializa o contador global
uint32_t OS_global_ticks = 0;


OSThread idleThread;
void main_idleThread() {
    while (1) {
        OS_onIdle();
    }
}

void OS_init(void *stkSto, uint32_t stkSize) {
    /* set the PendSV interrupt priority to the lowest level 0xFF */
    *(uint32_t volatile *)0xE000ED20 |= (0xFFU << 16);

    /* start idleThread thread */
    OSThread_start(&idleThread,
                   &main_idleThread,
                   stkSto, stkSize);
}

void OS_sched(void) {
    if (OS_readySet == 0U) { /* idle condition? */
    	OS_currIdx = 0U; /* the idle thread */
    } else {

		// alteração para o EDF
		uint32_t menor_dl = 0xFFFFFFFFU;
      	uint8_t menor_id = 0;

      	for(uint8_t i=1; i<OS_threadNum; i++){
        	if(OS_readySet & (1U <<(i - 1U))){
          		if(OS_thread[i]->deadline < menor_dl){
            	menor_dl = OS_thread[i]->deadline;
            	menor_id = i;
          		}
        	}
      	}
		// fim alteração para o EDF
		
    	/* anteriormente esse trecho era para prioridade estática 
		do{ // find the next ready thread
            OS_currIdx++;
            if(OS_currIdx == OS_threadNum){
            	OS_currIdx = 1;
            }
            OS_next = OS_thread[OS_currIdx];
    	}while((OS_readySet & (1U <<(OS_currIdx - 1U))) == 0 );
    }*/
		
    OS_next = OS_thread[OS_currIdx];
    /* trigger PendSV, if needed */
    if(OS_next != OS_curr){
    	*(uint32_t volatile *)0xE000ED04 = (1U << 28);
    }
}

void OS_run(void) {
    /* callback to configure and start interrupts */
    OS_onStartup();

    __disable_irq();
    OS_sched();
    __enable_irq();

    /* the following code should never execute */
    Q_ERROR();
}

void OS_tick(void) {
	// incrementa contador global
  OS_global_ticks++;
  
  uint8_t n = 0;
	for(n = 1U; n < OS_threadNum; n++){ /* cycle through every thread but the idle */
    if(OS_thread[n]->timeout != 0U){
      OS_thread[n]->timeout--;			/* decrease the timeout (por software)*/
			
      if(OS_thread[n]->timeout == 0U){
        
        // adicioção: tempo passou do tempo do periodo
        if(OS_thread[n]->periodo > 0U){
          // coloca o periodo da tarefa no contador
          OS_thread[n]->timeout = OS_thread[n]->period;
          // calcula o deadline
          OS_thread[n]->deadline = OS_global_ticks + OS_thread[n]->period;
        }
		// fim adição
		  
        OS_readySet |= (1U << (n-1U));	/* if the thread is ready mask the corresponding bit */
			}
		}
	}
}

void OS_delay(uint32_t ticks) {
    __asm volatile ("cpsid i");

    /* never call OS_delay from the idleThread */
    Q_REQUIRE(OS_curr != OS_thread[0]);

    OS_curr->timeout = ticks;
    OS_readySet &= ~(1U << (OS_currIdx - 1U));
    OS_sched();
    __asm volatile ("cpsie i");
 }

void OSThread_start(
    OSThread *me,
    OSThreadHandler threadHandler,
    void *stkSto, uint32_t stkSize
    uint8_t periodo) // adição da varíavel periodo
{
    /* round down the stack top to the 8-byte boundary
    * NOTE: ARM Cortex-M stack grows down from hi -> low memory
    */
    uint32_t *sp = (uint32_t *)((((uint32_t)stkSto + stkSize) / 8) * 8);
    uint32_t *stk_limit;

    /* thread number must be in ragne
    * and must be unused
    */
    Q_REQUIRE((OS_threadNum < Q_DIM(OS_thread)) && (OS_thread[OS_threadNum] == (OSThread *)0));

    *(--sp) = (1U << 24);  /* xPSR */
    *(--sp) = (uint32_t)threadHandler; /* PC */
    *(--sp) = 0x0000000EU; /* LR  */
    *(--sp) = 0x0000000CU; /* R12 */
    *(--sp) = 0x00000003U; /* R3  */
    *(--sp) = 0x00000002U; /* R2  */
    *(--sp) = 0x00000001U; /* R1  */
    *(--sp) = 0x00000000U; /* R0  */
    /* additionally, fake registers R4-R11 */
    *(--sp) = 0x0000000BU; /* R11 */
    *(--sp) = 0x0000000AU; /* R10 */
    *(--sp) = 0x00000009U; /* R9 */
    *(--sp) = 0x00000008U; /* R8 */
    *(--sp) = 0x00000007U; /* R7 */
    *(--sp) = 0x00000006U; /* R6 */
    *(--sp) = 0x00000005U; /* R5 */
    *(--sp) = 0x00000004U; /* R4 */

    /* save the top of the stack in the thread's attibute */
    me->sp = sp;

    /* round up the bottom of the stack to the 8-byte boundary */
    stk_limit = (uint32_t *)(((((uint32_t)stkSto - 1U) / 8) + 1U) * 8);

    /* pre-fill the unused part of the stack with 0xDEADBEEF */
    for (sp = sp - 1U; sp >= stk_limit; --sp) {
        *sp = 0xDEADBEEFU;
    }

    // adição
	me->periodo = periodo;
    me->timeout = periodo;
    me->deadline = OS_global_ticks + periodo;
	// fim adição

    /* register the thread with the OS */
    OS_thread[OS_threadNum] = me;
    /* make the thread ready to run */
    if (OS_threadNum > 0U) {
        OS_readySet |= (1U << (OS_threadNum - 1U));
    }
    OS_threadNum++;
}
/***********************************************/
void OS_onStartup(void) {
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / TICKS_PER_SEC);

    /* set the SysTick interrupt priority (highest) */
    NVIC_SetPriority(SysTick_IRQn, 0U);
}

void OS_onIdle(void) {
#ifdef NDBEBUG
    __WFI(); /* stop the CPU and Wait for Interrupt */
#endif
}


// funções adicionadas para o escalonador
void OS_wait_period(void){
    __disable_irq();
    OS_readySet &= ~(1U << (OS_currIdx - 1U));
    OS_sched();
    __enable_irq();
}

void yield(void) {
    __asm volatile ("cpsid i");
    OS_sched();
    __asm volatile ("cpsie i");
}

OSSemaphore::OSSemaphore(int16_t initialCount) : count(initialCount), inicio(0), fim(0){
    Q_REQUIRE(initialCount >= 0);
}

void OSSemaphore::wait(void){
    __asm volatile ("cpsid i");
    count--;

    if (count < 0){
        bloqueados[fim] = OS_curr;
        fim = (fim + 1) % 32;
        OS_readySet &= ~(1U << (OS_currIdx - 1U));
        OS_sched();
    }
    __asm volatile ("cpsie i");
}

void OSSemaphore::signal(void){
    __asm volatile ("cpsid i");
    count++;

    if (count <= 0){
        OSThread* tarefa = bloqueados[inicio];
        inicio = (inicio + 1) % 32;

        for (uint8_t i = 1; i < OS_threadNum; i++){
            if (OS_thread[i] == tarefa){
                OS_readySet |= (1U << (i - 1U));
                break;
            }
        }
        OS_sched();
    }
    __asm volatile ("cpsie i");
}
// fim funções adicionadas para o escalonador



// funções adicionadas para prod cons
void OSComunication::write(int32_t value){
    empty.wait(); 
    mutex.wait();

    // região crítica com fifo circular
    Buffer[inicio] = value;
    inicio = (inicio + 1) % n;

    mutex.signal();
    full.signal();
}

int32_t OSComunication::read(){ // correção de sintaxe
    full.wait();
    mutex.wait();

    // região crítica com fifo circular
    int32_t value = Buffer[fim];
    fim = (fim + 1) % n;

    mutex.signal();
    empty.signal();
    return value;
}
// fim fuções adicionadas para prod cons



}//fim namespace

void Q_onAssert(char const *module, int loc) {
    /* TBD: damage control */
    (void)module; /* avoid the "unused parameter" compiler warning */
    (void)loc;    /* avoid the "unused parameter" compiler warning */
    NVIC_SystemReset();
}

/***********************************************/
__attribute__ ((naked, optimize("-fno-stack-protector")))
void PendSV_Handler(void) {
__asm volatile (

    /* __disable_irq(); */
    "  CPSID         I                 \n"

    /* if (OS_curr != (OSThread *)0) { */
    "  LDR           r1,=_ZN4rtos7OS_currE       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  CBZ           r1,PendSV_restore \n"

    /*     push registers r4-r11 on the stack */
    "  PUSH          {r4-r11}          \n"

    /*     OS_curr->sp = sp; */
    "  LDR           r1,=_ZN4rtos7OS_currE       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  STR           sp,[r1,#0x00]     \n"
    /* } */

    "PendSV_restore:                   \n"
    /* sp = OS_next->sp; */
    "  LDR           r1,=_ZN4rtos7OS_nextE       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  LDR           sp,[r1,#0x00]     \n"

    /* OS_curr = OS_next; */
    "  LDR           r1,=_ZN4rtos7OS_nextE       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  LDR           r2,=_ZN4rtos7OS_currE       \n"
    "  STR           r1,[r2,#0x00]     \n"

    /* pop registers r4-r11 */
    "  POP           {r4-r11}          \n"

    /* __enable_irq(); */
    "  CPSIE         I                 \n"

    /* return to the next thread */
    "  BX            lr                \n"
    );
}
