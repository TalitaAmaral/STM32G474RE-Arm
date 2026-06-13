// miros.h

class OSComunication {
private:
    static const uint8_t n = 10;
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

// miros.cpp

void OSComunication::write(int32_t value){
    empty.wait(); 
    mutex.wait();

    // região crítica com fifo circular
    Buffer[inicio] = value;
    inicio = (inicio + 1) % n;

    mutex.signal();
    full.signal();
}

int32_t OSComunication::read(){
    full.wait();
    mutex.wait();

    // região crítica com fifo circular
    int32_t value = Buffer[fim];
    fim = (fim + 1) % n;

    mutex.signal();
    empty.signal();
    return value;
}
