PROGRAM_FILE=./src/main
PROGRAM_NAME=haversine_release

build: 
	gcc -Wall -O3 -o $(PROGRAM_NAME) $(PROGRAM_FILE).c
	gcc -Wall -O3 -o read_overhead_test src/read_overhead_main.c

read_overhead_test:

debug:
	gcc -Wall -g -o $(PROGRAM_NAME).out $(PROGRAM_FILE) 

run:
	./$(PROGRAM_NAME) data_10_flex.json data_10_answer.f64
