/*
 * COMMAND HEADER FILE
 *
 * Manage and parse commands from smallsh.c
 * */

#ifndef CMD_H
#define CMD_H

// Header files
#include <stdio.h>
#include <stdlib.h>

// Constants
#ifndef WORD_SIZE
#define WORD_SIZE 128
#endif

#ifndef MAX_COMPONENTS
#define MAX_COMPONENTS 518
#endif

// Command Struct
struct Cmd
{
	int numArgs;									// Number of args not including redir/bg process indicator
	char ** args;									// Array of char args ready for exec
	int bgProc;										// True/false is this a bg process
	int redirStdin;									// Should stdin be redirected
	int redirStdout;								// Should stdout be redirected
	char stdinFile[WORD_SIZE];						// Filename of stdin redirect
	char stdoutFile[WORD_SIZE];						// Filename of stdout redirect
};

// Function Prototypes
void initCmd(struct Cmd * command);					// Initialize command struct
void parseCmd(struct Cmd * command, char * line);	// Parse line received
void destroyCmd(struct Cmd * command);				// Free memory when done

#endif

