/*
 * macros.h - Macro preprocessing header
 * 
 * Defines structures and functions for macro preprocessing during assembly.
 */

#ifndef MACROS_H
#define MACROS_H

/*
 * LineNode - Single line in macro body
 */
typedef struct LineNode {
    char* line;                 /* Text content of the line */
    struct LineNode* next;      /* Pointer to next line in the macro */
}LineNode;

/*
 * Macro - Macro definition with name and body
 */
typedef struct Macro {
    char* name;                 /* Name of the macro */
    LineNode* lines_head;       /* Head of the linked list of macro body lines */
    struct Macro* next;         /* Pointer to next macro in the table */
} Macro;

/* Macro management functions */
void add_macro(const char* name);                           /* Creates new macro with given name */
Macro* find_macro(Macro* head,const char* name);           /* Searches for macro by name */
int is_macro_start(const char* line);                       /* Checks if line starts with "mcro " */
int is_macro_end(const char* line);                         /* Checks if line starts with "mcroend" */
int is_macro_call(const Macro* head,const char* line);     /* Checks if line is a macro call */

/* Line node management */
LineNode* new_line_node(const char* line);                 /* Creates new line node */
void add_line_to_macro(Macro* head,char* line);            /* Adds line to macro body */
void destroy(Macro* head);                                  /* Frees all macro memory */

/* Macro processing functions */
char* extract_macro_name(const char* line);                /* Extracts macro name from line */
void expand_macro(Macro* head,const char* name,FILE* file); /* Expands macro to file */
int is_valid_macro_name(const char *name);                 /* Validates macro name */

FILE* preprocessor(FILE* file,char* filename);             /* Main macro preprocessing function */

#endif /* MACROS_H */