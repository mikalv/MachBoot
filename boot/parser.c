/*
 * Copyright 2013, winocm. <winocm@icloud.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 *   Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 *   If you are going to use this software in any form that does not involve
 *   releasing the source to this project or improving it, let me know beforehand.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "genboot.h"

int util_parse( 
               char *input,     /* input string                */
               char **argv,     /* output array to hold tokens */
               char *delimiters,/* token delimiters            */
               int  max_tokens, /* max number of tokens (number of elements in argv) */
               char quote)       /* quote character for strings */
{
  char *ptr;
  int i = 0, nquote = 0;

  if (max_tokens < 1) 
  {
    return -1;
  }

  /* Save embedded blanks inside quoted strings */

  if (quote != 0) 
  {
    for (ptr = input; *ptr != (char) 0; ptr++) 
    {
      if (*ptr == quote) 
      {
        if (++nquote == 2) nquote = 0;
      }
      else 
        if (nquote == 1 && *ptr == ' ') *ptr = (char) -1;
    }
  }

  /* Parse the string, restoring blanks if required */

  if ((argv[0] = strtok(input, delimiters)) == NULL) return 0;
    
  i = 1;
  do 
  {
    if ((argv[i] = strtok(NULL, delimiters)) != NULL && quote != 0) 
      for (ptr = argv[i]; *ptr != (char) 0; ptr++) 
        if (*ptr == (char) -1) *ptr = ' ';
  } while (argv[i] != NULL && ++i < max_tokens);

  /* Return the number of tokens */

  return i;
}

static void command_parse(char* str)
{
    char* array[8] = {0};
    int i = 0, x = 0;

  if (strlen(str) < 3) /* if user simply pressed enter, ignore */
	 return;

    str += 2; /* '] ' */
    i = util_parse(str, array, " ,", 8, '"');

    while(gDispatch[x].function) {
        if(array[0]) {
            if(strcmp(gDispatch[x].name, array[0]) == 0) {
                if(gDispatch[x].function) {
                    i--;
                    gDispatch[x].function(i, array);
                    return;
                }
            }
        }
        x++;
    }

    printf("\x1b[01m" "?SYNTAX ERROR" "\x1b[00m" "\n");

    return;
}

extern int uart_getchar(void);
static void safe_gets(char *mod, char *str, int maxlen)
{
	register char *lp;
	register int c;
	char *refmod = mod; /* make copy of mod so we don't backspace over it later */
	char *strmax = str + maxlen - 1; /* allow space for trailing 0 */

	lp = str;

	while(*mod)
	{
		*lp++ = *mod;
		printf("%c", *mod);
		mod++;
	}

	for (;;) {
	    c = uart_getchar();
	    switch (c) {
    case -1:
      continue;
		case '\n':
		case '\r':
		    printf("\n");
		    *lp++ = 0;
		    return;

		case '\b':
		case '#':
		case '\177':
		    if (lp > (str + strlen(refmod))) {
			printf("\b \b");
			lp--;
		    }
		    continue;
		case '@':
		case 'u'&037:
		    lp = str;
		    printf("\n\r");
		    continue;
		default:
		    if (c >= ' ' && c < '\177') {
			if (lp < strmax) {
			    *lp++ = c;
			    printf("%c", c);
			}
		    }
	    }
	}
}

void command_prompt(void) {
    printf("Entering recovery mode, starting command prompt\n");
    while(1) {
        char buffer[256];
        safe_gets("] ", buffer, 256);
        command_parse(buffer);
    }
}
