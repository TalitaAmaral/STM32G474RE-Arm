#ifndef INC_MIROS_H_
#define INC_MIROS_H_


namespace rtos {
/* Thread Control Block (TCB) */
typedef struct {
    void *sp; /* stack pointer */
    uint32_t timeout; /* timeout delay down-counter */
    /* ... other attributes associated with a thread */
	
	// variáveis adicionadas
	uint32_t periodo;
	uint32_t deadline;
} OSThread;


// contador adicionado
extern uint32_t OS_global_tick;

// classe adicionada semaforo
class OSSemaphore{
public:
	explicit OSSemaphore(int16_t initialCount);
	void wait(void);
	void signal(void);

private:
	int16_t count;
	OSThread* bloqueados[32];
	uint8_t inicio;
	uint8_t fim;
};
//fim classe adicionada semaforo

// classe para prod cons
class OSComunication {
private:
    static const uint8_t n = 10; // correção de sintaxe
    int32_t Buffer[n];
    
    uint8_t inicio = 0; // escrita (produtor)
    uint8_t fim = 0;   // leitura (consumidor)

    OSSemaphore empty;
    OSSemaphore full;
    OSSemaphore mutex;

public:
    explicit OSComunication() : empty(n), full(0), mutex(1) {}
    void write(int32_t value);
    int32_t read(void);
};
//fim classe para prod cos


const uint16_t TICKS_PER_SEC = 100U;

typedef void (*OSThreadHandler)();

void OS_init(void *stkSto, uint32_t stkSize);

/* callback to handle the idle condition */
void OS_onIdle(void);

/* this function must be called with interrupts DISABLED */
void OS_sched(void);


// funções adicionadas
void OS_wait_period(void)
void yield(void);
// fim funções adicionadas


/* transfer control to the RTOS to run the threads */
void OS_run(void);

/* blocking delay */
void OS_delay(uint32_t ticks);

/* process all timeouts */
void OS_tick(void);

/* callback to configure and start interrupts */
void OS_onStartup(void);

void OSThread_start(
    OSThread *me,
    OSThreadHandler threadHandler,
    void *stkSto, uint32_t stkSize,
	uint8_t periodo); // adição da variável periodo

} // fim namespace rtos

#endif /* INC_MIROS_H_ */
