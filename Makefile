CC = gcc
CFLAGS = -Wall -g
TARGET = city_manager

all: city_manager monitor_reports

city_manager: main.o city_commands.o 
	$(CC) -o city_manager main.o city_commands.o 

monitor_reports: monitor_reports.c
	$(CC) -o monitor_reports monitor_reports.c

clean:
	rm -f *.o city_manager monitor_reports .monitor_pid