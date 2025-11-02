/*
 * input.c - File input operations
 * 
 * This file contains functions for reading and processing input files.
 * It provides a simple interface for reading lines from files with
 * proper buffer management and error handling.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* Added for strlen */

#include "input.h"

/*
 * Reads a single line from the specified file into the provided buffer.
 * The function handles end-of-file conditions and ensures the buffer
 * is properly null-terminated. Also validates that line length is not
 * more than 80 characters (excluding newline).
 */
int read_line(FILE *fp, char *buffer) {
    int len;
    
    if (fgets(buffer, MAX_LINE_LENGTH + 1, fp) == NULL) {
        return 0; /* EOF or error */
    }
    
    /* Check line length - should be max 80 chars (excluding newline) */
    len = strlen(buffer);
    
    if (len == MAX_LINE_LENGTH) {
        printf("Error: Line too long (max 80 characters allowed)\n");
        return 2; /* Return error */
    }
    
    return 1; /* success */
}

