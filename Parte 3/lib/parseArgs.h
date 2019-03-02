#ifndef PARSEARGS_H
#define PARSEARGS_H

/**

Arguments:
 'argVector' should be a vector of char* previously allocated with
 as many entries as 'vectorSize'
 'vectorSize' is the size of the above vector. A vector of size N allows up to
 N-1 arguments to be read; the entry after the last argument is set to NULL.
 'buffer' is a buffer with 'buffersize' bytes, which will be
 used to hold the strings of each argument.

Return value:
 The number of arguments that were read, or -1 if some error occurred.
*/

int parseArgs(char **argVector, int vectorSize, char *buffer, int bufferSize);

#endif
