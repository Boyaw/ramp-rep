CC = g++

CFLAGS = -g -Wall -pthread -std=c++17

TARGET = demo

all: $(TARGET) 

$(TARGET): $(TARGET).cpp $(TARGET).h 
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp 

clean:
	$(RM) $(TARGET)
