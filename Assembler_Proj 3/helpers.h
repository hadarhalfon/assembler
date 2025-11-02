/*
 * helpers.h - Helper functions header
 * 
 * This header file declares utility functions used throughout the assembler
 * for common operations such as memory management, string parsing,
 * symbol validation, and operand type checking.
 */

#ifndef HELPERS_H
#define HELPERS_H

#include <stddef.h>

/* Constants for maximum lengths */
#define MAX_NUM_LENGTH 4        /* Maximum length for numeric values */
#define MAX_SYMBOL_LENGTH 30    /* Maximum length for symbol names */

/* Memory management functions */
void* safe_malloc(size_t size, const char* error_msg);        /* Safe memory allocation with error handling */
char* create_filename(const char* base_name, const char* extension); /* Creates filename with extension */

/* String parsing and validation functions */
int space_skip(const char*line,int i);                         /* Skips whitespace and returns next position */
int is_symbol_definition(const char *line, int i);             /* Checks if line contains symbol definition */
int is_symbol(const char * line, int i);                       /* Validates if string is a legal symbol name */
int is_directive(const char *line, int i);                     /* Checks if line starts with a directive */
int is_number(char* line, int index);                          /* Validates if string represents a number */
int is_legal_data_or_matrix_initialization(const char *line, int i, int line_num); /* Validates .data/.mat syntax */
int is_legal_string(const char *line, int i, int line_num);   /* Validates .string directive syntax */
int is_legal_mat(const char * line, int i, int line_num);     /* Validates matrix definition syntax */
int contains_invalid_commas(const char *line, int index);      /* Checks for invalid comma usage in data */
int contains_invalid_matrix_commas(const char *line, int index); /* Checks for invalid comma usage in matrices */
int get_data_comma_error_type(const char *line, int index);    /* Determines specific comma error type */

/* Operand type checking functions */
int is_register(const char *line, int i, int line_num);       /* Validates register operand (r0-r7) */
int is_mat_operand(const char *line, int i, int line_num);    /* Validates matrix operand syntax */
int is_direct(const char *line, int i, int line_num);         /* Validates direct addressing operand */
int save_place(char *line, int line_num);                     /* Saves current position for error reporting */

#endif /* HELPERS_H */
