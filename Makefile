CC = gcc
CFLAGS = -Wall -g
TARGET = city_manager

all: city_manager monitor_reports scorer

city_manager: main.o city_commands.o 
	$(CC) -o city_manager main.o city_commands.o 

scorer: scorer.c
	$(CC) -o scorer scorer.c

monitor_reports: monitor_reports.c
	$(CC) -o monitor_reports monitor_reports.c


clean:
	rm -f *.o city_manager monitor_reports .monitor_pid scorer