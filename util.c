/* See LICENSE file for copyright and license details. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"


void 
sh(const char* cmd, char* output)
{
	FILE *fp;

  	/* Open the command for reading. */
  	fp = popen(cmd, "r");
  	if (fp == NULL)
    		die("Failed to run command");

  	/* Read the first line of output, if cannot then throw error */
  	if (fgets(output, sizeof(output), fp) == NULL) {
    		die("could not read commands output");
  	}

	output[strlen(output)-1] = '\0'; /* remove newling charachter */

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
