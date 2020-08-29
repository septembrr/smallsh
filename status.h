/*
 * STATUS HEADER FILE
 * */

#ifndef STATUS_H
#define STATUS_H

// Header files
#include <stdio.h>
#include <stdlib.h>

// Status Struct
struct Status
{
	int value;				// Status number - either exit status # or signal #
	int normalTerm;			// True false - was it normal termination? If not, must be signal
};

// Function prototypes
void initStatus(struct Status * status);
void changeStatus(struct Status * status, int childExitMethod);
void printStatus(struct Status * status);
int wasSignalTerm(struct Status * status);

#endif
