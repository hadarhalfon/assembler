/*
 * input.h - File input operations header
 * 
 * This header file declares functions for reading and processing input files.
 */

#ifndef INPUT_H
#define INPUT_H

#define MAX_LINE_LENGTH 81

/* File input functions */
int read_line(FILE *fp, char *buffer);  /* Reads a line from file into buffer */

#endif /* INPUT_H */