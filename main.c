#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "raylib.h"

#define WIDTH 64
#define HEIGHT 32

unsigned short opcode;
unsigned char memory[4096];
unsigned char registers[16];
unsigned short index_register;
unsigned short program_counter;
unsigned char graphics[WIDTH * HEIGHT];
unsigned char delay_timer;
unsigned char sound_timer;
unsigned short stack[16];
unsigned short stack_pointer;
unsigned char key[16];
size_t bufferSize = 0;
bool drawFlag = true;

#define BYTE 8
#define VF 15 // carry VF register

void emulate_cycle();

char *read_file_into_buffer(char *filename)
{
    FILE *file = fopen(filename, "rb");

    fseek(file, 0, SEEK_END);
    long size = ftell(file);

    rewind(file);
    char *buffer = malloc(size + 1);

    size_t read = fread(buffer, 1, size, file);
    buffer[read] = '\0';

    fclose(file);

    bufferSize = read;

    return buffer;
}

void init(char *filepath)
{
    program_counter = 0x200;
    opcode = 0;
    index_register = 0;
    stack_pointer = 0;

    char *buffer = read_file_into_buffer(filepath);

    unsigned char chip8_fontset[80] =
        {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };

    for (int i = 0; i < 80; i++)
    {
        memory[i] = chip8_fontset[i];
    }

    for (int i = 0; i < bufferSize; i++)
    {
        memory[i + 512] = buffer[i];
    }
}

void render()
{
    const int screenWidth = WIDTH * 10;
    const int screenHeight = HEIGHT * 10;

    const int rectSize = 10;
    const int cyclesPerFrame = 5;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(24);

    while (!WindowShouldClose())
    {
        key[0x1] = IsKeyDown(KEY_ONE);
        key[0x2] = IsKeyDown(KEY_TWO);
        key[0x3] = IsKeyDown(KEY_THREE);
        key[0xC] = IsKeyDown(KEY_FOUR);

        key[0x4] = IsKeyDown(KEY_Q);
        key[0x5] = IsKeyDown(KEY_W);
        key[0x6] = IsKeyDown(KEY_E);
        key[0xD] = IsKeyDown(KEY_R);

        key[0x7] = IsKeyDown(KEY_A);
        key[0x8] = IsKeyDown(KEY_S);
        key[0x9] = IsKeyDown(KEY_D);
        key[0xE] = IsKeyDown(KEY_F);

        key[0xA] = IsKeyDown(KEY_Z);
        key[0x0] = IsKeyDown(KEY_X);
        key[0xB] = IsKeyDown(KEY_C);
        key[0xF] = IsKeyDown(KEY_V);

        emulate_cycle();

        if (drawFlag)
        {
            BeginDrawing();

            ClearBackground(BLACK);
            DrawFPS(10, 10);

            for (size_t index = 0; index < WIDTH * HEIGHT; index++)
            {
                if (graphics[index] != 0)
                {
                    size_t x = index % WIDTH;
                    size_t y = index / WIDTH;
                    DrawRectangle(x * rectSize, y * rectSize, rectSize, rectSize, WHITE);
                }
            }
            drawFlag = false;

            EndDrawing();
        }
    }

    CloseWindow();
}

void emulate_cycle()
{
    opcode = memory[program_counter] << BYTE | memory[program_counter + 1];

    short x = (opcode & 0x0F00) >> 8;
    short y = (opcode & 0x00F0) >> 4;
    short kk;

    switch (opcode & 0xF000)
    {
    case 0xA000: // LD I, addr
        index_register = (opcode & 0x0FFF);
        program_counter += 2;

        break;
    case 0xB000: // JP V0, addr
        program_counter = (opcode & 0x0FFF) + registers[0];

        break;
    case 0xC000: // RND Vx byte
        kk = opcode & 0x00FF;
        int r = (rand() % 256) & kk;

        registers[x] = r;

        program_counter += 2;

        break;
    case 0xD000: // DRW Vx Vy, nibble
        unsigned short height = opcode & 0x000F;
        unsigned short pixel;

        registers[VF] = 0;

        for (size_t col = 0; col < height; col++)
        {
            pixel = memory[index_register + col];

            for (int row = 0; row < 8; row++)
            {
                unsigned short bitmask = pixel & (0x80 >> row); // 128 >> 0 = 1000 0000, 128 >> 1 (64) = 0100 0000, etc.

                int px = (registers[x] + row) % 64;
                int py = (registers[y] + col) % 32;

                unsigned short pixel_position = px + py * 64;

                // unsigned short pixel_position = registers[x] + row + ((registers[y] + col) * 64);

                if (bitmask != 0)
                {
                    if (graphics[pixel_position] == 1)
                        registers[VF] = 1;
                    graphics[pixel_position] ^= 1;
                }
            }
        }

        drawFlag = true;
        program_counter += 2;

        break;
    case 0xE000:
        switch (opcode & 0x00FF)
        {
        case 0x009E: // SKP Vx
            if (registers[x] < 16 && key[registers[x]])
                program_counter += 4;
            else
                program_counter += 2;

            break;
        case 0x00A1: // SKNP Vx
            if (registers[x] < 16 && key[registers[x]])
                program_counter += 2;
            else
                program_counter += 4;

            break;
        default:
            printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
        }

        break;
    case 0xF000:
        switch (opcode & 0x00FF)
        {
        case 0x0007: // Fx07 - LD Vx, DT
            registers[x] = delay_timer;
            break;
        case 0x000A: // Fx0A - LD Vx, K
            bool pressed = false;

            for (int i = 0; i < 16; i++)
            {
                if (key[i])
                {
                    registers[x] = i;
                    pressed = true;
                    break;
                }
            }

            if (!pressed)
                return;

            break;

            break;
        case 0x0015: // Fx15 - LD DT, Vx
            delay_timer = registers[x];
            break;
        case 0x0018: // Fx18 - LD ST, Vx
            sound_timer = registers[x];
            break;
        case 0x001E: // Fx1E - ADD I, Vx
            index_register += registers[x];
            break;
        case 0x0029: // Fx29 - LD F, Vx
            index_register = 5 * registers[x];

            break;
        case 0x0033: // Fx33 - LD B, Vx
            memory[index_register] = registers[x] / 100;
            memory[index_register + 1] = registers[x] / 10 % 10;
            memory[index_register + 2] = registers[x] % 100 % 10;

            break;
        case 0x0055: // Fx55 - LD [I], Vx
            for (size_t index = 0; index <= x; index++)
            {
                memory[index_register + index] = registers[index];
            }

            break;
        case 0x0065: // Fx65 - LD Vx, [I]
            for (size_t index = 0; index <= x; index++)
            {
                registers[index] = memory[index_register + index];
            }
            break;
        default:
            printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
        }

        program_counter += 2;

        break;
    case 0x0000:
        switch (opcode & 0x00FF)
        {
        case 0x00E0: // clear the screen
            memset(graphics, 0, sizeof graphics);

            program_counter += 2;
            break;

        case 0x00EE: // return from subroutine
            program_counter = stack[--stack_pointer];
            // stack_pointer--;

            break;
        default:
            printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
        }

        break;
    case 0x1000: // JP addr
        program_counter = (opcode & 0x0FFF);

        break;
    case 0x2000: // CALL addr
        stack[stack_pointer++] = program_counter + 2;
        program_counter = (opcode & 0x0FFF);

        break;
    case 0x3000: // SE Vx byte
        kk = (opcode & 0x00FF);

        if (registers[x] == kk)
            program_counter += 2;

        program_counter += 2;

        break;
    case 0x4000: // SNE Vx byte
        kk = (opcode & 0x00FF);

        if (registers[x] != kk)
            program_counter += 2;

        program_counter += 2;

        break;
    case 0x5000: // SE Vx Vy
        if (registers[x] == registers[y])
            program_counter += 2;

        program_counter += 2;

        break;
    case 0x6000: // LD Vx byte
        kk = (opcode & 0x00FF);
        registers[x] = kk;

        program_counter += 2;
        break;
    case 0x7000: // ADD Vx byte
        kk = (opcode & 0x00FF);
        registers[x] += kk;

        program_counter += 2;
        break;
    case 0x8000:
        switch (opcode & 0x000F)
        {
        case 0x0000: // LD Vx Vy
            registers[x] = registers[y];
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
            registers[VF] = registers[y] >= registers[x];
            registers[x] = registers[y] - registers[x];

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
    case 0x9000: // SNE Vx Vy
        if (registers[x] != registers[y])
            program_counter += 2;

        program_counter += 2;

        break;
    default:
        printf("Unknown opcode: 0x%X\n", opcode);
    }

    if (delay_timer > 0)
        delay_timer--;

    if (sound_timer > 0)
        sound_timer--;
}

int main(int argc, char *argv[])
{
    init(argv[1]);

    render();
    return 0;
}
