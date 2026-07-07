#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"

unsigned short opcode;
unsigned char memory[4096];
unsigned char registers[16];
unsigned short index_register;
unsigned short program_counter;
unsigned char graphics[64 * 32];
unsigned char delay_timer;
unsigned char sound_timer;
unsigned short stack[16];
unsigned short stack_pointer;
unsigned char key[16];

#define BYTE 8
#define VF 15 // carry VF register

char *read_file_into_buffer(char *filename, size_t *bufferSize)
{
    FILE *file = fopen("program", "r");

    fseek(file, 0, SEEK_END);
    long size = ftell(file);

    rewind(file);
    char *buffer = malloc(size + 1);

    size_t read = fread(buffer, 1, size, file);
    buffer[read] = '\0';

    fclose(file);

    (*bufferSize) = read;

    return buffer;
}

void init()
{
    program_counter = 0x200;
    opcode = 0;
    index_register = 0;
    stack_pointer = 0;

    size_t bufferSize;
    char *buffer = read_file_into_buffer("program", &bufferSize);

    // for (int i = 0; i < 80; i++)
    // {
    //     memory[i] = chip8_fontset[i];
    // }

    for (int i = 0; i < bufferSize; i++)
    {
        memory[i + 512] = buffer[i];
    }
}

void emulate_cycle()
{
    opcode = memory[program_counter] << BYTE | memory[program_counter + 1];

    switch (opcode & 0xF000)
    {
    case 0xA000: // LD I, addr
        break;
    case 0xB000: // JP V0, addr
        break;
    case 0xC000: // RND Vx byte
        break;
    case 0xD000: // DRW Vx Vy, nibble
        break;
    case 0xE000:
        switch (opcode & 0x00FF)
        {
        case 0x009E: // SKP Vx
            break;
        case 0x00A1: // SKNP Vx
            break;
        default:
            printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
        }
        break;
    case 0xF000:
        switch (opcode & 0x00FF)
        {
        case 0x0007: // Fx07 - LD Vx, DT
            break;
        case 0x000A: // Fx0A - LD Vx, K
            break;
        case 0x0015: // Fx15 - LD DT, Vx
            break;
        case 0x0018: // Fx18 - LD ST, Vx
            break;
        case 0x001E: // Fx1E - ADD I, Vx
            break;
        case 0x0029: // Fx29 - LD F, Vx
            break;
        case 0x0033: // Fx33 - LD B, Vx
            break;
        case 0x0055: // Fx55 - LD [I], Vx
            break;
        case 0x0065: // Fx65 - LD Vx, [I]
            break;
        default:
            printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
        }
        break;
    case 0x000:
        switch (opcode & 0x000F)
        {
        case 0x0000: // clear the screen
            break;
        case 0x000E: // return from subroutine
            break;
        default:
            printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
        }
        break;
    case 0x1000: // JP addr
        program_counter = opcode & 0x0FFF;
        break;
    case 0x2000: // CALL addr
        stack[stack_pointer] = program_counter;
        stack_pointer++;
        program_counter = opcode & 0x0FFF;

        break;
    case 0x3000:
        // SE Vx byte
        break;
    case 0x4000:
        // SNE Vx byte
        break;
    case 0x5000:
        // SE Vx Vy
        break;
    case 0x6000:
        // LD Vx byte
        break;
    case 0x7000:
        // ADD Vx byte
        break;
    case 0x8000:
        short x = (opcode & 0x0F00) >> 8;
        short y = (opcode & 0x00F0) >> 4;

        switch (opcode & 0x000F)
        {
        case 0x0000: // LD Vx Vy
            break;
        case 0x0001: // OR Vx Vy
            registers[x] = registers[x] | registers[y];
            break;
        case 0x0002: // AND Vx Vy
            registers[x] = registers[x] & registers[y];
            break;
        case 0x0003: // XOR Vx Vy
            registers[x] = registers[x] ^ registers[y];
            break;
        case 0x0004: // ADD Vx Vy
            if (registers[y] > 255 - registers[x])
            {
                registers[VF] = 1;
            }
            else
            {
                registers[VF] = 0;
            }

            registers[x] += registers[y];
            break;
        case 0x0005: // SUB Vx Vy
            if (registers[x] > registers[y])
            {
                registers[VF] = 1;
            }
            else
            {
                registers[VF] = 0;
            }

            registers[x] -= registers[y];
            break;
        case 0x0006: // SHR Vx {, Vy}
            registers[VF] = registers[x] & 1;
            registers[x] >>= 1;
            break;
        case 0x0007: // SUBN Vx Vy
            break;
        case 0x000E: // SHL Vx {, Vy}
            registers[VF] = registers[x] >> 7;
            registers[x] <<= 1;
            break;
        default:
            printf("Unknown opcode [0x8000]: 0x%X\n", opcode);
        }

        program_counter += 2;

        break;
    case 0x9000:
        // SNE Vx Vy
        break;
    default:
        printf("Unknown opcode: 0x%X\n", opcode);
    }
}

int main()
{
    return 0;
}
