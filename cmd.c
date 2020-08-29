/*
 * COMMAND IMPLEMENTATION FILE
 *
 * Manage and parse commands from smallsh.c
 * */

// Header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmd.h"

// Constants
#ifndef WORD_SIZE
#define WORD_SIZE 128
#endif

#ifndef MAX_COMPONENTS
#define MAX_COMPONENTS 518
#endif

// Global to handle foreground mode
unsigned int fgMode = 0;

/*
 * INITIALIZE COMMAND STRUCT
 * */
void initCmd(struct Cmd * command)
{
	// Set all arguments to 0 or NULL, as needed
	command->numArgs = 0;
	command->args = NULL;
	command->bgProc = 0;

	command->redirStdin = 0;
	command->redirStdout = 0;

	memset(command->stdinFile, '\0', sizeof(command->redirStdin));
	memset(command->stdoutFile, '\0', sizeof(command->redirStdout));
}


/*
 * PARSE COMMAND
 * */
void parseCmd(struct Cmd * command, char * line)
{
	// Variables to parse command
	char * components[MAX_COMPONENTS];	// All words/components in original command
	char buffer[WORD_SIZE];				// Buffer to hold each word
	int args = 0;						// Number of final arguments, not including redir/bg character
	int i = 0;							// Counter
	char * word = NULL;					// For strtok, getting each word
	pid_t shellPid = getpid();			// The shell's pid, for replacement

	// Use strtok() to parse words from line received
	word = strtok(line, " \n");

	// Get all words from command
	while(word != NULL)
	{
		components[args] = word;
		args++;
		word = strtok(NULL, " \n");
	}

	// Now loop through and act on arguments
	for(i = 0; i < args; i++)
	{
		// stdout redirection
		if(!strcmp(">", components[i]))
		{
			strcpy(command->stdoutFile, components[i+1]);
			command->redirStdout = 1;
		}

		// stdin redirection
		if(!strcmp("<", components[i]))
		{
			strcpy(command->stdinFile, components[i+1]);
			command->redirStdin = 1;
		}

		// Replace $$ with process ID
		char * pidLoc;
		while(pidLoc = strstr(components[i], "$$"))
		{
			memset(buffer, '\0', sizeof(buffer));

			// Change $$ to be format specifier instead
			*pidLoc = '%';
			*(++pidLoc) = 'd';

			// Reassign as component
			snprintf(buffer, sizeof(buffer), components[i], shellPid);
			components[i] = buffer;
		}
	}

	// Check if background process
	if( (args > 0) && (!strcmp("&", components[args-1])) )
	{
		// Only if not in foreground mode
		if(!fgMode)
			command->bgProc = 1;

		// Adjust number of args to match real args
		args--;
	}

	// Adjust number of args to match real args
	if(command->redirStdin)
		args = args - 2;
	if(command->redirStdout)
		args = args - 2;
	command->numArgs = args;

	// Create array of args used in exec()
	// Command is first arg, and last arg is NULL
	command->args = malloc(sizeof(char *) * (args + 1));
	if(command->args == NULL) exit(5);

	// Loop through and assign args
	for(i = 0; i < args; i++)
	{
		command->args[i] = malloc(sizeof(char) * WORD_SIZE);
		if(command->args[i] == NULL) exit(5);
		memset(command->args[i], '\0', sizeof(char) * WORD_SIZE);
		strcpy(command->args[i], components[i]);
	}

	// Final arg should be NULL
	command->args[args] = NULL;
}

/*
 * DESTROY COMMAND STRUCT
 * */
void destroyCmd(struct Cmd * command)
{
	int i = 0;	// Loop through

	// Free all memory in a loop
	for(i = 0; i < command->numArgs; i++)
	{
		if(command->args[i] != NULL)
		{
			free(command->args[i]);
			command->args[i] = NULL;
		}
	}

	// Free array of args itself
	free(command->args);
	command->args = NULL;
}

