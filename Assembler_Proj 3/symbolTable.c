/*
 * symbolTable.c - Symbol table management
 * 
 * This file contains functions for managing the symbol table, which stores
 * information about symbols (labels) defined in the assembly code. It handles:
 * - Creating and adding new symbols
 * - Searching for existing symbols
 * - Updating symbol values and types
 * - Memory cleanup for symbol tables
 * - Data symbol address adjustments
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symbolTable.h"
#include "helpers.h"

/*
 * Allocates and initializes a new Symbol structure with the given name.
 * The value is initialized to 0, type to 0, and next pointer to NULL.
 */
Symbol* new_symbol(const char *name) {
    Symbol *tmp = NULL;
    tmp = (Symbol*)safe_malloc(sizeof(Symbol), "Error in add_symbol()");
    tmp->name = (char*)safe_malloc(strlen(name) + 1, "Error in memory allocation for symbol name");
    strcpy(tmp->name, name);
    tmp->value = 0;
    tmp->type = 0; /* Initialize type to 0 */
    tmp->next = NULL;
    return tmp;
}

/*
 * Adds the final instruction counter (ICF) to all data symbol values.
 * This adjusts data symbol addresses to their final positions in memory
 * after the first pass is complete.
 */
void update_data_symbols_value(Symbol *head, int ICF) {
    while (head != NULL) {
        if (head->type == 1) {
            head->value += ICF;
        }
        head = head->next;
    }
}

/*
 * set_type - Sets the type of a symbol
 */
void set_type(Symbol *symbol, int type) {
    symbol->type = type;
}

/*
 * set_value - Sets the value (address) of a symbol
 */
void set_value(Symbol *symbol, int value) {
    symbol->value = value;
}

/*
 * add_symbol - Adds a new symbol to the symbol table
 */
Symbol* add_symbol(Symbol** head, Symbol** tail, const char* name) {
    Symbol* tmp = new_symbol(name);
    if (*head == NULL) {
        *head = tmp;
        *tail = tmp;
    } else {
        (*tail)->next = tmp;
        *tail = tmp;
    }
    return tmp;
}

/*
 * destroy_symbol_table - Frees all memory allocated for a symbol table
 */
void destroy_symbol_table(Symbol *head) {
    if (head == NULL) {
        return;
    }
    while (head != NULL) {
        Symbol *tmp = head;
        head = head->next;
        free(tmp->name);
        free(tmp);
    }
}

/*
 * search_symbol - Searches for a symbol by name
 */
Symbol* search_symbol(Symbol *head,const char *name) {
    Symbol *tmp = head;
    while (tmp != NULL) {
        if (strcmp(tmp->name, name) == 0) {
            return tmp;
        }
        tmp = tmp->next;
    }
    return NULL;
}