CC = gcc
CFLAGS += -g -Wall

foo: foo.c
	$(CC) $(CFLAGS) $^ -o $@

all: foo

gdb: foo
	gdb $^ --tui
clean:
	$(RM) foo
