CC = gcc
CFLAGS = -Wall -g
TARGET = city_manager

all: $(TARGET)

$(TARGET): main.o city_commands.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o city_commands.o

main.o: main.c city_commands.h
	$(CC) $(CFLAGS) -c main.c

city_commands.o: city_commands.c city_commands.h
	$(CC) $(CFLAGS) -c city_commands.c

clean:
	rm -f *.o $(TARGET)