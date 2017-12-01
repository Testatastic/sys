#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

int
main()
{
	//static char readbuf[41];

	const char *fileA, *fileB, *fileC;
	int rv;

	
		/*warnx("No arguments - running on \"testfile\"");*/
		fileA = "testA";
		fileB = "testB";
		fileC = "testC";


	rv = meld(fileA,fileB, fileC);
	if (rv<0) {
		err(1, "MeldError");
	}
return 0;
}
