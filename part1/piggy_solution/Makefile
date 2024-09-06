PROGRAM_NAME = 8086_sim

FILES = ./src/*.c 

CFLAGS = -Wall -std=c11 
OFLAGS += -O3

build: 
	gcc $(CFLAGS) $(OFLAGS) -o $(PROGRAM_NAME) $(FILES)

run:
	./$(PROGRAM_NAME)

clean: 
	rm $(PROGRAM_NAME)

debug: 
	gcc $(CFLAGS)-g $(FILES)
	lldb a.out
