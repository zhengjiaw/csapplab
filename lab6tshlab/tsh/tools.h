#ifndef TOOLS_H
#define TOOLS_H
#include "base.h"
#define MAXPARSE 64
/*
 * parseline - Parse the command line and build the argv array.
 *
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.
 */
int parseline(const char *buf, char **argv);
size_t redirect(char **argv);
size_t pipeCmdline(const char *cmdline, char **argv);
#endif