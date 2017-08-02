#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

int GPIOExport(int pin);

int GPIOUnexport(int pin);

int GPIODirection(int pin, int dir);

int GPIOEdge(int pin, int dir);

int GPIORead(int pin);

int GPIOPoll(int pin);

int GPIOWrite(int pin, int value);
