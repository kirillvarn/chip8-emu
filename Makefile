main.exe:
	@gcc main.c -Iinclude -Llib -lraylib -lgdi32 -lwinmm -lm -o main.exe
