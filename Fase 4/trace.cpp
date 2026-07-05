
#include "trace.h"
#include "stm32g4xx.h"   // DWT->CYCCNT


trace_blob_t g_trace;

//(1 tick = 1 ciclo de CPU)
static inline uint32_t trace_now(void) {
    return DWT->CYCCNT;
}

void trace_init(uint32_t clock_hz) {
    g_trace.magic    = TRACE_MAGIC;
    g_trace.clock_hz = clock_hz;
    g_trace.count    = 0u;
    g_trace.capacity = TRACE_CAPACITY;
    for (uint32_t i = 0u; i < TRACE_MAX_TASKS; ++i) {
        g_trace.names[i][0] = '\0';
    }
}

void trace_name(uint8_t id, const char *nm) {
    if (id >= TRACE_MAX_TASKS || nm == 0) {
        return;
    }
    uint32_t i = 0u;
    for (; (i < TRACE_NAME_LEN - 1u) && (nm[i] != '\0'); ++i) {
        g_trace.names[id][i] = nm[i];
    }
    g_trace.names[id][i] = '\0';
}

void trace_evt(uint8_t code, uint8_t id) {
    uint32_t n = g_trace.count;
    if (n >= g_trace.capacity) {
        return;
    }
    g_trace.events[n].cycles = trace_now();
    g_trace.events[n].code   = code;
    g_trace.events[n].id     = id;
    g_trace.events[n].arg    = 0u;
    g_trace.count = n + 1u;
}
