/*
 * order.h - Instruction/Order header
 * 
 * This header file defines the Order structure and related functions
 * for managing assembly instructions. It includes opcode tables,
 * addressing mode constants, and instruction processing functions.
 */

#ifndef ORDER_H
#define ORDER_H

#include "symbolTable.h"
#include "word.h"

/*
 * Order - Represents an assembly instruction
 * 
 * An order represents a single assembly instruction with its operands,
 * addressing modes, and associated binary words.
 */
typedef struct Order {
    int IC;                    /* Instruction counter value */
    int opcode;                /* Instruction opcode (0-15) */
    int operand1;              /* First operand addressing mode */
    int operand2;              /* Second operand addressing mode */
    int number_of_words;       /* Total words needed for instruction */
    int number_of_operands;    /* Number of operands (0-2) */
    int symbol_flag;           /* Flag for symbol resolution */
    char *symbol_name1;        /* First operand symbol name */
    char *symbol_name2;        /* Second operand symbol name */
    Word* word;                /* Pointer to instruction words */
    struct Order *next;        /* Pointer to next order in list */
}Order;

/* Addressing mode constants */
#define ADDR_IMMEDIATE 0       /* Immediate addressing (#value) */
#define ADDR_DIRECT    1       /* Direct addressing (symbol) */
#define ADDR_MATRIX    2       /* Matrix addressing ([symbol]) */
#define ADDR_REGISTER  3       /* Register addressing (r0-r7) */

#define NUM_OPCODES 16         /* Number of supported instructions */

/* External arrays for operand validation */
const int legal_dst_modes[NUM_OPCODES][4];  /* Legal destination modes per instruction */
const int legal_src_modes[NUM_OPCODES][4];  /* Legal source modes per instruction */

/* Instruction processing functions */
int opcode_in_decimal(const char* line,int index,int line_num);  /* Converts instruction name to opcode */
int addressing_method(const char* line,int i,int line_num);      /* Determines operand addressing mode */
int number_of_lines(int operand1,int operand2);                 /* Calculates instruction size */
int number_of_operands(int op);                                 /* Returns number of operands for opcode */

/* Order management functions */
Order* new_order(int op);                                      /* Creates new order */
Order* add_order(Order** head,Order** tail,int op);           /* Adds order to list */
int update_symbol_operands(Order* order_head, Symbol* symbol_head,Symbol** external_head); /* Resolves symbols */
void destroy_order(Order* head);                               /* Frees order list */
void add_word_to_order(Order* order, Word* word);             /* Adds word to order */

#endif /* ORDER_H */