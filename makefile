all: main.c functions.c functions.h dataTypes.h
	gcc -Wall -Wextra --pedantic main.c functions.c -lm -o img_filter

clean:
