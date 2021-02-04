all: main

main: src/*
	$(CC) src/* -o main -Wall -W -pedantic -ansi

dev: src/*
	$(CC) src/* -o main -Wall -W -pedantic -ansi -fsanitize=address

clean:
	rm main -f

