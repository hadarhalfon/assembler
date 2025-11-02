/*
 * macros.c - Macro preprocessing system
 * 
 * This file implements a macro preprocessing system for the assembler.
 * It handles macro definition, expansion, and validation. The system supports:
 * - Macro definition with "mcro" and "mcroend" directives
 * - Macro name validation against reserved instruction and directive names
 * - Macro expansion during preprocessing
 * - Memory management for macro structures
 * 
 * Macro syntax:
 *   mcro macro_name
 *   ... macro body lines ...
 *   mcroend
 * 
 * Macro call syntax:
 *   macro_name
 * 
 */

#include <stdio.h>
#include "macros.h"
#include "input.h"

#include <stdlib.h>
#include <string.h>

/* Global macro list head - stores all defined macros */
Macro* head = NULL;

/*
 * Creates a new Macro structure with the given name and adds it to the
 * beginning of the global macro linked list. The macro is initialized
 * with an empty lines list.
 */
void add_macro(const char* name) {
    Macro *new_macro = (Macro *) malloc(sizeof(Macro));
    if (new_macro==NULL) {
        printf("Memory error in add_macro()\n");
        exit(1);
    }
    new_macro->name = malloc(strlen(name) + 1);
    if (!new_macro->name) {
        printf("Memory allocation failed for macro name\n");
        exit(1);
    }
    strcpy(new_macro->name, name);

    /* Initialize macro structure */
    new_macro->lines_head = NULL;
    new_macro->next = NULL;
    
    /* Add to beginning of global macro list */
    if (head == NULL) {
        head = new_macro;
    }
    else {
        new_macro->next = head;
        head = new_macro;
    }
}

/*
 * Traverses the macro linked list looking for a macro with the given name.
 * Returns the first matching macro found, or NULL if not found.
 */
Macro* find_macro(Macro* head,const char* name) {
    while (head != NULL) {
        if (strcmp(head->name, name) == 0) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

/* 
 * Determines if the given line begins with "mcro " (macro start directive).
 * This identifies the beginning of a macro definition block.
 */
int is_macro_start(const char* line) {
    return strncmp(line,"mcro ",5) == 0;
}

/*
 * Determines if the given line begins with "mcroend" (macro end directive).
 * This identifies the end of a macro definition block.
 */
int is_macro_end(const char* line) {
    return strncmp(line,"mcroend",7) == 0;
}

/*
 * Extracts the first word from the line and checks if it matches
 * any defined macro name. Handles leading whitespace and extracts
 * the macro name up to the first space or newline.
 */
int is_macro_call(const Macro* head,const char* line) {
    char macro_name[MAX_LINE_LENGTH];
    int i = 0;
    int j = 0;

    /* Skip leading spaces and tabs */
    while (line[i] == ' ' || line[i] == '\t') {
        i++;
    }

    /* Extract macro name (up to first space, tab, or newline) */
    while (line[i] != '\0' && line[i] != ' ' && line[i] != '\t' && line[i] != '\n' && line[i] != '\r' && j < MAX_LINE_LENGTH - 1) {
        macro_name[j] = line[i];
        i++;
        j++;
    }
    macro_name[j] = '\0';

    /* Check if this macro name exists in the macro list */
    while (head != NULL) {
        if (strcmp(head->name, macro_name) == 0) {
            return 1;
        }
        head = head->next;
    }
    return 0;
}

/*
 * Allocates and initializes a new LineNode structure with the given line.
 * The line is copied to avoid memory issues with the original string.
 */
LineNode* new_line_node(const char* line) {
    LineNode *tmp = NULL;
    tmp = (LineNode *)malloc(sizeof(LineNode));
    if(tmp == NULL) {
        printf("Memory error in new_line_node()\n");
        return NULL;
    }
    tmp->line = (char *) malloc(strlen(line)+1);
    if(tmp->line == NULL) {
        printf("Memory error in new_line_node()\n");
        free(tmp);
        return NULL;
    }
    strcpy(tmp->line, line);
    tmp->next = NULL;

    return tmp;
}

/*
 * Adds the given line to the end of a macro's lines list.
 * If the macro has no lines yet, creates the first line node.
 * Otherwise, traverses to the end and adds the new line.
 */
void add_line_to_macro(Macro* head,char* line) {
    if (head->lines_head == NULL) {
        /* First line in the macro */
        head->lines_head = new_line_node(line);
    }
    else {
        /* Add to end of existing lines list */
        LineNode *temp = head->lines_head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_line_node(line);
    }
}

/*
 * Recursively frees all Macro structures in a linked list, including
 * their LineNode lists and associated strings. Sets all pointers to NULL.
 */
void destroy(Macro* head) {
    if(head == NULL)
        return;
    while(head != NULL) {
        Macro* tmp = head;
        head = head->next;
        
        /* Free all lines in this macro */
        while (tmp->lines_head != NULL) {
            LineNode* temp = tmp->lines_head;
            tmp->lines_head = tmp->lines_head->next;
            free(temp->line);
            free(temp);
        }
        free(tmp->name);
        free(tmp);
    }
}

/*
 * Extracts the first word from a line, skipping leading whitespace.
 * The extracted name is stored in a static buffer and returned.
 * Handles various whitespace characters and line endings.
 */
char* extract_macro_name(const char* line) {
    static char macro_name[MAX_LINE_LENGTH];
    int i = 0;
    int j = 0;

    /* Skip leading spaces and tabs */
    while (line[i] == ' ' || line[i] == '\t') {
        i++;
    }

    /* Extract macro name (up to first space, tab, or newline) */
    while (line[i] != '\0' && line[i] != ' ' && line[i] != '\t' && line[i] != '\n' && line[i] != '\r' && j < MAX_LINE_LENGTH - 1) {
        macro_name[j] = line[i];
        i++;
        j++;
    }
    macro_name[j] = '\0';

    return macro_name;
}

/*
 * Finds the specified macro in the macro list and writes all its
 * body lines to the given file. If the macro is not found, prints
 * an error message and returns without writing anything.
 */
void expand_macro(Macro* head,const char* name,FILE* file) {
    Macro* tmp = find_macro(head,name);
    LineNode *curr;
    
    if (tmp == NULL) {
        printf("Error: Macro '%s' not found\n", name);
        return;
    }
    
    /* Write all lines in the macro body to the file */
    curr = tmp->lines_head;
    while (curr != NULL) {
        fputs(curr->line, file);
        curr = curr->next;
    }
}

/*
 * Checks if the given macro name is valid by ensuring it doesn't
 * conflict with instruction names or directive names. Validates that
 * the name is not NULL or empty.
 */
int is_valid_macro_name(const char *name) {
    /* Array of instruction names that cannot be used as macro names */
    const char *instructions[] = {
        "mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec",
        "jmp", "bne", "jsr", "red", "prn", "rts", "stop"
    };

    /* Array of directive names that cannot be used as macro names */
    const char *directives[] = {
        ".data", ".string", ".mat", ".extern", ".entry"
    };

    int i;

    /* Check if name is NULL or empty */
    if (name == NULL || strlen(name) == 0) {
        return 0;
    }

    /* Check against instruction names */
    for (i = 0; i < 16; i++) {
        if (strcmp(name, instructions[i]) == 0) {
            return 0; /* Invalid - matches instruction name */
        }
    }

    /* Check against directive names */
    for (i = 0; i < 5; i++) {
        if (strcmp(name, directives[i]) == 0) {
            return 0; /* Invalid - matches directive name */
        }
    }

    return 1; /* Valid macro name */
}

/*
 * preprocessor - Main macro preprocessing function
 * 
 * Processes an input file and performs macro expansion. Reads the file
 * line by line and handles:
 * - Macro definitions (mcro/mcroend blocks)
 * - Macro calls (expands macros inline)
 * - Regular assembly code (passes through unchanged)
 * 
 * Creates a temporary .am file with all macros expanded, then reopens
 * it for reading by the assembler.
 * 
 * Parameters:
 *   file - Input file pointer (assembly source)
 *   filename - Base filename without extension
 * 
 * Returns:
 *   File pointer to the preprocessed file (.am), or NULL on error
 * 
 * Error handling:
 *   - Reports invalid macro names
 *   - Reports extra characters after macro directives
 *   - Returns NULL if any errors occur during preprocessing
 * 
 * Memory management:
 *   - Allocates temporary filename
 *   - Creates and manages temporary .am file
 *   - Cleans up macro list after processing
 */
FILE* preprocessor(FILE* file,char* filename) {
    int in_macro = 0;          /* Flag indicating if we're inside a macro definition */
    int error_flag = 0;        /* Flag for tracking preprocessing errors */
    char line[MAX_LINE_LENGTH]; /* Buffer for reading lines */
    char *filename1 = NULL;    /* Temporary filename for .am file */
    FILE *fp1;                 /* File pointer for temporary .am file */
    char *name = NULL;         /* Macro name being processed */

    /* Allocate memory for temporary filename */
    filename1 = (char*)malloc(strlen(filename) + 5);
    if (!filename1) {
        printf("Memory allocation error in preproccessor function\n");
        exit(1);
    }
    strcpy(filename1, filename);
    strcat(filename1, ".am");

    /* Create temporary file for preprocessed output */
    fp1 = fopen(filename1 ,"w");
    if (fp1 == NULL) {
        printf("Memory allocation error in preproccessor function: can't create file %s\n", filename1);
        free(filename1);
        exit(1);
    }

    /* Process input file line by line */
    while (fgets(line,MAX_LINE_LENGTH,file) !=NULL) {
        int i = 5;  /* Skip "mcro " prefix when processing macro start */
        name = NULL;

        if (is_macro_start(line)) {
            /* Start of macro definition */
            in_macro = 1;
            
            /* Skip spaces after "mcro " */
            while (line[i] == ' ') {
                i += 1;
            }
            
            /* Extract macro name */
            name = extract_macro_name(line + i);

            /* Validate that there are no extra characters after macro name */
            i += strlen(name);
            while (line[i] == ' ') {
                i++;
            }
            if (line[i] != '\0' && line[i] != '\n' && line[i] != '\r') {
                printf("chars after macro definition\n");
                error_flag = 1;
            }
            
            /* Validate macro name */
            if (!is_valid_macro_name(name)) {
                printf("Error: macro name not valid\n");
                error_flag = 1;
            }
            
            /* Add macro to global list */
            add_macro(name);
        }
        else if (in_macro == 1 && !is_macro_end(line)) {
            /* Inside macro definition - add line to current macro */
            add_line_to_macro(head,line);
        }
        else if (in_macro == 1 && is_macro_end(line)) {
            /* End of macro definition */
            i = 7;  /* Skip "mcroend" prefix */
            
            /* Validate that there are no extra characters after "mcroend" */
            while (line[i] == ' ') {
                i += 1;
            }
            if (line[i] != '\0' && line[i] != '\n' && line[i] != '\r') {
                printf("chars after macro end\n");
                error_flag = 1;
            }
            in_macro = 0;  /* Exit macro definition mode */
        }
        else if (is_macro_call(head,line)) {
            /* Macro call - expand the macro inline */
            char* macro_name = extract_macro_name(line);
            expand_macro(head, macro_name, fp1);
        }
        else {
            /* Regular assembly line - pass through unchanged */
            fputs(line,fp1);
        }
    }

    /* Close the temporary file */
    fclose(fp1);

    /* Clean up macro list */
    destroy(head);
    head = NULL; /* Reset global head */

    /* Reopen the file for reading by the assembler */
    fp1 = fopen(filename1, "r");
    if (fp1 == NULL) {
        printf("Error: can't reopen file %s for reading\n", filename1);
        free(filename1);
        exit(1);
    }

    /* Clean up and return */
    free(filename1);
    if (error_flag) 
        return NULL;  /* Return NULL if any errors occurred */
    return fp1;
}