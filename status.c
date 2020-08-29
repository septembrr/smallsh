/*
 * STATUS IMPLEMENTATION FILE
 * */

// Header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "status.h"

/*
 * INITIALIZE STATUS STRUCT
 * */
void initStatus(struct Status * status)
{
	status->value = 0;
	status->normalTerm = 1;
}

/*
 * CHANGE STATUS
 * */
void changeStatus(struct Status * status, int childExitMethod)
{
	// Check status with macros
	// It exited normally:
	if (WIFEXITED(childExitMethod))
	{
		status->value = WEXITSTATUS(childExitMethod);
		status->normalTerm = 1;
	}
	// It exited with a signal:
	else if (WIFSIGNALED(childExitMethod))
	{
		status->value = WTERMSIG(childExitMethod);
		status->normalTerm = 0;
	}
}

/*
 * PRINT STATUS
 * */
void printStatus(struct Status * status)
{
	// Print current status message depending on most recent type of status
	if(status->normalTerm)
	{
		printf("exit value %d\n", status->value);
		fflush(stdout);
	}
	else
	{
		printf("terminated by signal %d\n", status->value);
		fflush(stdout);
	}
}

/*
 * IS THE STATUS SIGNAL TERMINATION?
 * */
int wasSignalTerm(struct Status * status)
{
	// Check if status is normal termination or not
	if(status->normalTerm)
		return 0;
	else
		return 1;
}
