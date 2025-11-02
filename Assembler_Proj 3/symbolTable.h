/*
 * symbolTable.h - Symbol table header
 * 
 * This header file defines the Symbol structure and related functions
 * for managing the symbol table, which stores information about
 * symbols (labels) defined in the assembly code.
 */

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

/*
 * Symbol - Represents a symbol in the symbol table
 * 
 * A symbol represents a label defined in the assembly code.
 * Each symbol has a name, an address value, a type, and a pointer
 * to the next symbol in the linked list.
 */
typedef struct SymbolTable {
    char *name;                    /* Symbol name (label) */
    int value;                     /* Symbol address in memory */
    int type;                      /* Symbol type: 1=data, 2=code, 3=entry, 4=extern */
    struct SymbolTable *next;      /* Pointer to next symbol in list */
}Symbol;

/* Symbol table management functions */
Symbol* new_symbol(const char *name);                           /* Creates new symbol */
void update_data_symbols_value(Symbol *head, int ICF);         /* Updates data symbol addresses */
void set_type(Symbol *symbol, int type);                       /* Sets symbol type */
void set_value(Symbol *symbol, int value);                     /* Sets symbol address */
Symbol* add_symbol(Symbol** head, Symbol** tail, const char* name); /* Adds symbol to table */
void destroy_symbol_table(Symbol *head);                       /* Frees symbol table */
Symbol* search_symbol(Symbol *head,const char *name);          /* Searches for symbol by name */

#endif /* SYMBOLTABLE_H */