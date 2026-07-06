unsigned short opcode;
unsigned char memory[4096];
unsigned char registers[16];
unsigned short index_register;
unsigned short program_counter;
unsigned char[64*32];
unsigned char delay_timer;
unsigned char sound_timer;
unsigned short stack[16];
unsigned short stack_pointer;
unsigned char key[16];

