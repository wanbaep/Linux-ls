CC = gcc
EXEC = ls

all: ls.c
	$(CC) -o $(EXEC) $^

clean:
	rm -rf $(EXEC)
