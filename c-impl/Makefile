CC = g++

CFLAGS = -g -Wall -pthread -std=c++17

TARGET = demo

$(TARGET): $(TARGET).cpp $(TARGET).h 
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp 

debug: debug_ramp.cpp
	$(CC) $(CFLAGS) -o debug debug_ramp.cpp

clean:
	$(RM) $(TARGET) debug
