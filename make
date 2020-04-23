
SRC = main.c functions.c 
OBJ = main.o functions.o
PROG = img_filter

$(PROG): $(OBJ)
	gcc $(OBJ) -o $(PROG)
	
$(OBJ): $(SRC)


