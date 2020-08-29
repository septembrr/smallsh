// smallsh
/***************************************************************************************/

// Header files
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>

// Custom header files
#include "linkedList.h"
#include "cmd.h"
#include "status.h"
#include "sigHandlers.h"

// Constants
#ifndef MAX_LINE_SIZE
#define MAX_LINE_SIZE 2049
#endif

// Function prototypes
void prompt(char * line, const int LINE_SIZE);
void ss_exit(struct LinkedList * procs);
void ss_cd(struct Cmd * command);
int ss_redir_stdin(char * file);
int ss_redir_stdout(char * file);
void check_bg_procs(struct LinkedList * procs);

// Global foreground mode
extern unsigned int fgMode;

int main()
{
	// Set up signals
	// SIGINT
	struct sigaction SIGINT_action = {0};
	SIGINT_action.sa_handler = catchSIGINT;
	sigfillset(&SIGINT_action.sa_mask);
	sigaction(SIGINT, &SIGINT_action, NULL);

	// SIGTSTP
	struct sigaction SIGTSTP_action = {0};
	SIGTSTP_action.sa_handler = catchSIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	// For getting each command's components
	struct Cmd command;

	// Shell state helpers
	struct LinkedList bgProcs;
	initList(&bgProcs);
	pid_t curPid;
	int childExitMethod;
	pid_t shellPid = getpid();

	// Shell status manager
	struct Status status;
	initStatus(&status);

	// Helper variables
	int i = 0;
	char lineBuf[MAX_LINE_SIZE];
	int result = 0;

	// Loop through until exit
	while(1)
	{
		// Check for background processes
		check_bg_procs(&bgProcs);

		// Get next command
		prompt(lineBuf, MAX_LINE_SIZE);

		// Parse command into struct
		initCmd(&command);
		parseCmd(&command, lineBuf);

		// If no command given, or it's a comment, go to next prompt
		if( (command.args[0] == NULL) || (!strcmp("#", command.args[0])) || ('#' == command.args[0][0]) )
		{
			destroyCmd(&command);
			continue;
		}

		// exit
		if(!strcmp("exit", command.args[0]))
		{
			destroyCmd(&command);
			ss_exit(&bgProcs);
			break;
		}

		// cd
		if(!strcmp("cd", command.args[0]))
		{
			ss_cd(&command);
			destroyCmd(&command);
			continue;
		}

		// status
		if(!strcmp("status", command.args[0]))
		{
			printStatus(&status);
			destroyCmd(&command);
			continue;
		}

		// Command requested not overridden in smallsh
		// Proceed to pass to fork()
		curPid = fork();
		result = 0;

		switch(curPid)
		{
			// Error with spawning process
			case -1:
				perror("ERROR: Spawn Error\n");
				exit(1);
				break;
			// CHILD PROCESS
			case 0:
				// SIGINT Updates
				// Update signal handler for foreground processes
				if(command.bgProc)
					SIGINT_action.sa_handler = SIG_IGN;
				else
					SIGINT_action.sa_handler = SIG_DFL;

				sigfillset(&SIGINT_action.sa_mask);
				sigaction(SIGINT, &SIGINT_action, NULL);

				// SIGTSTP Updates
				SIGTSTP_action.sa_handler = SIG_IGN;
				sigaction(SIGTSTP, &SIGTSTP_action, NULL);

				// Redirection Setup
				// Use bitwise OR to amass any error messages into result
				if(command.redirStdout)
					result |= ss_redir_stdout(command.stdoutFile);
				else if(command.bgProc)
					result |= ss_redir_stdout("/dev/null");
				if(command.redirStdin)
					result |= ss_redir_stdin(command.stdinFile);
				else if(command.bgProc)
					result |= ss_redir_stdin("/dev/null");

				// If any errors were made, exit
				if(result) exit(1);

				// EXEC!
				execvp(command.args[0], command.args); 
				
				// If here, problem with exec()
				printf("%s: no such file or directory\n", command.args[0]);
				fflush(stdout);
				destroyCmd(&command);
				exit(1);
				break;
			// PARENT PROCESS
			default:
				// If it's a background process
				if(command.bgProc)
				{
					printf("background pid is %d\n", (int)curPid);
					fflush(stdout);

					// Add to bg process linked list
					pushList(&bgProcs, curPid);
				}
				// Otherwise it's a foreground process
				else
				{
					// Set up mask to block SIGTSTP
					sigset_t signal;
					sigemptyset(&signal);
					sigaddset(&signal, SIGTSTP);

					// Use sigprocmask while waiting
					sigprocmask(SIG_BLOCK, &signal, NULL);
					int result = -1;

					// Loop keeping waiting in case waitpid returns error
					// This fixes problem with waitpid errors resulting in zombies
					while(result == -1)
					{
						result = waitpid(curPid, &childExitMethod, 0);
					}
				
					// remove mask
					sigprocmask(SIG_UNBLOCK, &signal, NULL);

					// Update status and print messages accordingly
					changeStatus(&status, childExitMethod);
					if(wasSignalTerm(&status))
						printStatus(&status);
				}

				// Clean up
				destroyCmd(&command);
				break;
		}
	}
	
	// Clean up bg process linked list at end of program
	freeList(&bgProcs);
	
	return 0;
}


/**********************************************************************************************/
/* GENERAL SHELL HELPERS */

/*
 * COMMAND LINE PROMPT
 * */
void prompt(char * line, const int LINE_SIZE)
{
	// Helper variables
	int charsInput = -2;
	char * newLine = NULL;
	size_t bufferSize = 0;

	// Prompt for next command
	// In loop to account for signal interrupts
	while(1)
	{
		printf(": ");
		fflush(stdout);

		// Get newline, checking for errors
		charsInput = getline(&newLine, &bufferSize, stdin);
		if (charsInput == -1)
			clearerr(stdin);
		else
			break;
	}

	// Assign to memory allocated, and free buffer
	memset(line, '\0', sizeof(char) * LINE_SIZE);
	strcpy(line, newLine);
	free(newLine);
	newLine = NULL;
}

/*
 * EXIT SMALLSH
 * Clean up any background processes still running
 * */
void ss_exit(struct LinkedList * procs)
{
	// Initialize iter to loop through bg procs
	struct ListIter iter;
	initListIter(&iter, procs);

	// Find and kill all bg procs
	while(listIterHasNext(&iter))
	{
		kill(listIterNext(&iter), SIGTERM);
	}
}

/*
 * SMALLSH CHANGE DIRECTORY
 * */
void ss_cd(struct Cmd * command)
{
	// If no arguments passed to cd, go to home
	if(command->args[1] == NULL)
	{
		char * home = getenv("HOME");
		chdir(home);
	}
	// Otherwise go to directory specified
	else
	{
		chdir(command->args[1]);
	}
}

/*
 * REDIRECT STDIN FILE DESCRIPTOR
 * */
int ss_redir_stdin(char * file)
{
	// Helper variables
	int sourceFD, result;

	// Open new file descriptor
	sourceFD = open(file, O_RDONLY);

	// If error in opening
	if (sourceFD == -1)
	{
		printf("cannot open %s for input\n", file);
		fflush(stdout);
		return -1;
	}

	// Assign file descriptor to new location
	result = dup2(sourceFD, 0);

	// If error in reassigning
	if (result == -1)
	{
		printf("cannot redirect to %s for output\n", file);
		fflush(stdout);
		return -1;
	}

	// otherwise return success
	return 0;
}

/*
 * REDIRECT STDOUT FILE DESCRIPTOR
 * */
int ss_redir_stdout(char * file)
{
	// Helper variables
	int targetFD, result;

	// Open new file descriptor
	targetFD = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	// If error in opening
	if (targetFD == -1)
	{
		printf("cannot open %s for output\n", file);
		fflush(stdout);
		return -1;
	}

	// Assign file descriptor to new location
	result = dup2(targetFD, 1);

	// If error in reassigning
	if (result == -1)
	{
		printf("cannot redirect to %s for output\n", file);
		fflush(stdout);
		return -1;
	}
	
	// Otherwise return success
	return 0;
}

/*
 * CHECK FOR COMPLETED BACKGROUND PROCESSES
 * */
void check_bg_procs(struct LinkedList * procs)
{
	// Initialize iterator
	struct ListIter iter;
	initListIter(&iter, procs);

	// Initialize list of procs to remove
	struct LinkedList toRemove;
	initList(&toRemove);

	// Helper variables
	int childExitMethod = -5;
	pid_t childPid = -5;
	int exitStatus, signo;

	// Check current elements
	while(listIterHasNext(&iter))
	{
		childPid = waitpid(listIterNext(&iter), &childExitMethod, WNOHANG);

		// If child process receives value, means it has completed
		if(childPid)
		{
			// Add it to list to remove
			pushList(&toRemove, childPid);

			// Print message
			printf("background pid %d is done: ", (int)childPid);
			fflush(stdout);

			// Include message with status
			// Normal termination:
			if (WIFEXITED(childExitMethod))
			{
				exitStatus = WEXITSTATUS(childExitMethod);
				printf("exit value %d\n", exitStatus);
				fflush(stdout);
			}
			// Signal termination
			else if (WIFSIGNALED(childExitMethod))
			{
				signo = WTERMSIG(childExitMethod);
				printf("terminated by signal %d\n", signo);
				fflush(stdout);
			}
		}
	}
	
	// Now remove completed processes from list
	initListIter(&iter, &toRemove);
	while(listIterHasNext(&iter))
	{
		removeFromList(procs, listIterNext(&iter));
	}

	// Free linked list when done
	freeList(&toRemove);
}
