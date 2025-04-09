TARGET = prog

PREF_SRC = ./src/
PREF_OBJ = ./obj/

# CFLAGS = -O0 -g -Wall
CC = g++

SRC = $(wildcard $(PREF_SRC)*.cpp)
OBJ = $(patsubst $(PREF_SRC)%.cpp, $(PREF_OBJ)%.o, $(SRC))


$(TARGET) : $(OBJ)
	$(CC) -g $(OBJ) -o $(TARGET)

$(PREF_OBJ)%.o : $(PREF_SRC)%.cpp
	$(CC) -c $< -o $@


.PHONY : clean
clean :
	rm -f $(PREF_OBJ)*.o $(TARGET)
