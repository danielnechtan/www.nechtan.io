/*
 * Copyright 2018 int16h <int16h@openbsd.space>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal 
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
*/

#include <stdio.h>
#include <string.h>

static char password[8] = "shiloh32";

char *
gets (char *s)
{
    char * ch = s;
    int k;
	
    while ((k = getchar ()) != '\n') {
        if (k == EOF) {
            if (ch == s || !feof(stdin)) 
                return NULL;
            break;
        }
        *ch++ = k;
    }
		
    *ch = '\0';
		
    return s; 
}

void checkpass(){
	char userpass[9];
	int ret;
	gets(userpass);
	if((ret = strcmp(password, userpass)) == 0){
		printf("\n%s is correct!\n", userpass);
	} else {
		printf("\n%s is incorrect!\n", userpass);
		exit(1);
	}
	printf("\nSecret stuff\n");
}

int
main(void)
{
	printf("\nWelcome to Cryogenix\n");
	printf("Password: ");
	checkpass();
}
