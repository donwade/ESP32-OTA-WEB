

#include <Arduino.h>
#include <RTClib.h>

#include "common.h"

//---------------------------------------------------------------
// my trace function

//https://medium.com/@turman1701/va-list-in-c-exploring-ft-printf-bb2a19fcd128

void trace(const char *fname, uint32_t lineNo, ...)
{
    va_list args, args2;
	char buffer[100];
	char newfmt[100];
	
    va_start(args, lineNo);

	sprintf(newfmt, "%s:%d %s", fname, lineNo, va_arg(args, char *));

	// get the arg of the arg .
	va_start(args2, args);

	// args2 points the the format specifer of the 
	//     second format specifier.
	
	// advance arg2 to point to the list of variables of the
	//   second format specifier.
	
	va_arg(args2, char *);  // this causes args2 to move one char pointer in size
	
    vsprintf(buffer, newfmt, args2);
	printf("TRACE %s", buffer);  // user provides \n

	va_end(args2);	
    va_end(args);
	return;
}

