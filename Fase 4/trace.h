#ifndef TRACE_H_
#define TRACE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TRACE_MAGIC      0x54524331u // "TRC1"
#define TRACE_MAX_TASKS  8u
#define TRACE_NAME_LEN   12u
#define TRACE_CAPACITY   1024u // nro maximo de eventos (one-shot)

enum {
    TR_CREATE = 1u,
    TR_RUN    = 2u,
    TR_READY  = 3u,
    TR_BLOCK  = 4u,
    TR_TICK   = 5u
};

/* 8 bytes */
typedef struct {
    uint32_t cycles;  /* DWT->CYCCNT no instante do evento */
    uint8_t  code;    /* TR_*                              */
    uint8_t  id;      /* indice da tarefa                  */
    uint16_t arg;     /* reservado                         */
} trace_evt_t;

/* bloco cabecalho + nomes + eventos*/
typedef struct {
    uint32_t    magic;
    uint32_t    clock_hz;
    uint32_t    count;
    uint32_t    capacity;
    char        names[TRACE_MAX_TASKS][TRACE_NAME_LEN];
    trace_evt_t events[TRACE_CAPACITY];
} trace_blob_t;

extern trace_blob_t g_trace; // buffer para analise

void trace_init(uint32_t clock_hz);
void trace_name(uint8_t id, const char *nm);
void trace_evt (uint8_t code, uint8_t id);

#ifdef __cplusplus
}
#endif

#endif /* TRACE_H_ */
