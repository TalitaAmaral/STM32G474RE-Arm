//miros.h

class OSComunication{
private:
	uint8_t n = 10;
	int32_t Buffer[10];
	OSSemaphore s(n);
	uint8_t spaces = n;

public:
	explicit OSComunication(){}
	void write(int32_t value);
	int32_t read(void);
};

//miros.cpp

void OSComunication::write(int32_t value){
	int32_t *ptr = Buffer;
	s.wait();
	for(uint8_t i = (10 - spaces); i > 0; i--){
		ptr[i] = ptr[i - 1];
	} ptr[0] = value;
	spaces--;
	s.signal();
}

uint32_t OSComunication::read(){
	int32_t *ptr = Buffer;
	int32_t value;
	s.wait();
	value = ptr[9 - spaces];
	spaces++;
	s.signal();
	return value;
}