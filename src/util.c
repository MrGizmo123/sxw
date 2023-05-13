/* See LICENSE file for copyright and license details. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"


void 
sh(const char* cmd, char* output, int output_size)
{
	FILE *fp;
	
	/* will add pipe to remove the newline char at the end of output */
	char final_command[256]; 
	memset(final_command, 0, sizeof(final_command));
	strcat(final_command, cmd);
	strcat(final_command, " | tr -d '\\n'");
	
  	/* Open the command for reading. */
  	fp = popen(final_command, "r");
  	if (fp == NULL)
    		die("Failed to run command");

  	/* Read the first line of output, if cannot then throw error */
  	if (fgets(output, output_size, fp) == NULL) {
    		die("could not read commands output");
  	}

	//output[strlen(output) - 1] = '\0'; /* remove newline charachter */

  	/* close */
  	pclose(fp);
}

void
die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	exit(1);
}

void *
ecalloc(size_t nmemb, size_t size)
{
	void *p;

	if (!(p = calloc(nmemb, size)))
		die("calloc:");
	return p;
}
