/* xsh_hello.c - xsh shell print hello */

#include <xinu.h>

/*------------------------------------------------------------------------
 * xsh_hello  -  shell command prints hello
 *------------------------------------------------------------------------
 */

shellcmd xsh_hello(int nargs, char *args[])
{
	hello();

	return 0;
}
