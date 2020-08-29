/*
 * SIGNAL HANDLERS IMPLEMENTATION FILE
 * */

// Header files
#include <unistd.h>
#include "cmd.h"

// Global Foreground Mode
// Comes from cmd.h library
extern unsigned int fgMode;

/*
 * CATCH SIGINT
 * */
void catchSIGINT(int signo)
{
	// Newline to push prompt to next line
	write(STDOUT_FILENO, "\n", 1);
}

/*
 * CATCH SIGTSTP
 * */
void catchSIGTSTP(int signo)
{
	char * onMsg = "\nEntering foreground-only mode (& is now ignored)\n";
	char * offMsg = "\nExiting foreground-only mode\n";
	
	// Print message depending on foreground mode
	if(fgMode)
		write(STDOUT_FILENO, offMsg, 30);
	else
		write(STDOUT_FILENO, onMsg, 50);
	
	// Flip foreground mode using bitwise operator
	fgMode = ~fgMode;
}

