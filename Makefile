main.exe:
	@gcc main.c -Iinclude -Llib -lraylib -lgdi32 -lwinmm -lm -o main.exe


CTAGS = ctags
SRCS = $(shell cmd /c "dir /s /b *.c *.h")

tags:
	@$(CTAGS) $(SRCS)

