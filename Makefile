CC=gcc
LIB_DIR= ./lib
INC_DIR= ./include
BIN_DIR= ./bin
SRC_DIR= ./src

all:	regra1 regra2 regra3 

regra1: $(BIN_DIR)/support.o
	ar crs $(LIB_DIR)/libsupport.a $(BIN_DIR)/support.o

regra2: $(SRC_DIR)/cthread.c 
	$(CC) -c $(SRC_DIR)/cthread.c -L$(LIB_DIR) -lsuppport -Include -Wall
	mv cthread.o $(BIN_DIR)
	

regra3:  $(BIN_DIR)/cthread.o
	ar crs $(LIB_DIR)/libcthread.a $(BIN_DIR)/cthread.o $(BIN_DIR)/support.o

clean:
	##rm $(BIN_DIR)/cthreadcontext.o
	rm $(BIN_DIR)/cthread.o
	rm $(LIB_DIR)/libsupport.a
	rm $(LIB_DIR)/libcthread.a
		
