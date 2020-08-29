/*
 * SIGNAL HANDLERS HEADER FILE
 * */

#ifndef SIGNAL_HANDLERS_H
#define SIGNAL_HANDLERS_H

// Header files
#include <signal.h>
#include <unistd.h>
#include "cmd.h"

// Signal Handlers
void catchSIGINT(int signo);
void catchSIGTSTP(int signo);

#endif
