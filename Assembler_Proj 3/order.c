/*
 * order.c - Instruction/Order management
 * 
 * This file contains functions for managing Order structures, which represent
 * assembly instructions. It handles:
 * - Opcode validation and conversion
 * - Addressing mode determination
 * - Instruction size calculation
 * - Order list management
 * - Operand validation and processing
 * - External symbol resolution
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "order.h"
#include "helpers.h"
#include "decode.h"

/* Instruction opcode table - maps instruction names to their numeric codes */
const char *ORDERS_TABLE[16] = {
    "mov","cmp","add","sub","lea",
    "clr","not","inc","dec","jmp",
    "bne","jsr","red","prn","rts","stop"
};

/* Legal source operand addressing modes for each instruction */
const int legal_src_modes[NUM_OPCODES][4] = {
    /* mov */   {1, 1, 1, 1},  /* Immediate, Direct, Matrix, Register */
    /* cmp */   {1, 1, 1, 1},
    /* add */   {1, 1, 1, 1},
    /* sub */   {1, 1, 1, 1},
    /* lea */   {0, 1, 1, 0},  /* Only Direct and Matrix allowed */
    /* clr */   {0, 0, 0, 0},  /* No source operand */
    /* not */   {0, 0, 0, 0},
    /* inc */   {0, 0, 0, 0},
    /* dec */   {0, 0, 0, 0},
    /* jmp */   {0, 0, 0, 0},
    /* bne */   {0, 0, 0, 0},
    /* jsr */   {0, 0, 0, 0},
    /* red */   {0, 0, 0, 0},
    /* prn */   {0, 0, 0, 0},
    /* rts */   {0, 0, 0, 0},
    /* stop */  {0, 0, 0, 0}
};

/* Legal destination operand addressing modes for each instruction */
const int legal_dst_modes[NUM_OPCODES][4] = {
    /* mov */   {0, 1, 1, 1},  /* No immediate for destination */
    /* cmp */   {1, 1, 1, 1},  /* All modes allowed for comparison */
    /* add */   {0, 1, 1, 1},
    /* sub */   {0, 1, 1, 1},
    /* lea */   {0, 1, 1, 1},
    /* clr */   {0, 1, 1, 1},
    /* not */   {0, 1, 1, 1},
    /* inc */   {0, 1, 1, 1},
    /* dec */   {0, 1, 1, 1},
    /* jmp */   {0, 1, 1, 1},
    /* bne */   {0, 1, 1, 1},
    /* jsr */   {0, 1, 1, 1},
    /* red */   {0, 1, 1, 1},
    /* prn */   {1, 1, 1, 1},  /* All modes allowed for printing */
    /* rts */   {0, 0, 0, 0},  /* No destination operand */
    /* stop */  {0, 0, 0, 0}
};

/*
 * Searches the ORDERS_TABLE for a matching instruction name and returns
 * its corresponding numeric opcode (0-15). Handles both 3 and 4 character
 * instruction names.
 */
int opcode_in_decimal(const char* line,int index,int line_num) {
    int i;
    for (i = 0; i<16 ; i++) {
        if (strncmp(line+index,ORDERS_TABLE[i],3)==0 || strncmp(line+index,ORDERS_TABLE[i],4)==0) {
            return i;
        }
    }
    printf("Error: Invalid opcode '%s' at line %d\n", line + index, line_num);
    return -1;
}

/*
 * addressing_method - Determines the addressing mode of an operand
 */
int addressing_method(const char* line,int i,int line_num) {
    if (is_direct(line,i,line_num) == 1 || is_direct(line,i,line_num) == 2) {
        return 0;  /* Immediate addressing (#value) */
    }
    if (is_register(line,i,line_num)) {
        return 3;  /* Register addressing (r0-r7) */
    }
    if (is_mat_operand(line,i,line_num)) {
        return 2;  /* Matrix addressing (symbol[][]) */
    }
    if (is_symbol(line,i)) {
        return 1;  /* Direct addressing (symbol) */
    }
    return -1;  /* Invalid addressing mode */
}

/*
 * number_of_lines - Calculates the number of words needed for an instruction
 */
int number_of_lines(int operand1,int operand2) {
    int lines = 1;  /* Base instruction word */
    if (operand1 == 0 || operand1 == 1)
        lines+= 1;  /* Immediate or direct addressing needs one extra word */
    else if (operand1 == 2)
        lines += 2; /* Matrix addressing needs two extra words */
    else if (operand1 == 3) {
        lines += 1; /* Register addressing needs one extra word */
        if (operand2 == 3)
            return lines; /* If both operands are registers, they share one word */
    }
    if (operand2 == 0 || operand2 == 1 || operand2 == 3) /* here they cant be both 3 */
        lines += 1;
    else if (operand2 == 2)
        lines += 2;
    return lines;
}


/*
 * Returns the number of operands for a given opcode.
 */
int number_of_operands(int op) {
    if (op > -1 && op <  5)
        return 2;
    if (op > 4 && op < 14)
        return 1;
    return 0;
}

/*
 * Creates a new Order structure and initializes all fields to default values.
 * Sets opcode, initializes operands to 0, and sets up memory pointers.
 */
Order* new_order(int op) {
    Order *tmp = (Order*)safe_malloc(sizeof(Order), "Memory allocation failed in new_order");
    tmp->opcode = op;
    tmp->operand1 = 0;
    tmp->operand2 = 0;
    tmp->symbol_flag = 0;
    tmp->symbol_name1 = NULL;  /* Initialize to NULL */
    tmp->symbol_name2 = NULL;  /* Initialize to NULL */
    tmp->word = NULL;
    tmp->IC = 0; /* Initialize IC to 0 */
    tmp->next = NULL;
    return tmp;
}


/*
 * Adds a new order to the end of the linked list.
 * Handles both empty list and non-empty list cases.
 */
Order* add_order(Order** head,Order** tail,int op) { /* adding to the end */
    Order* tmp;
    tmp = new_order(op);
    if (*head == NULL) {
        *head = tmp;  /* First element in list */
        *tail = tmp;
        return tmp;
    }
    (*tail)->next = tmp;  /* Add to end of existing list */
    *tail = tmp;
    return tmp;
}

int update_symbol_operands(Order* order_head, Symbol* symbol_head,Symbol** external_head) {
    Symbol* extern_tail = NULL;
    Order* curr_order = order_head;
    Symbol* symbol;
    Symbol* symbol2;
    Word* first_data_word;
    Word* curr_word2;
    Symbol* new_ext;
    Symbol* new_ext2;
    int error_flag = 0;

    /* Find the end of external symbols list for appending */
    if (*external_head != NULL) {
        extern_tail = *external_head;
        while (extern_tail->next != NULL) {
            extern_tail = extern_tail->next;
        }
    }

    /* Process each order that has symbols to resolve */
    while (curr_order != NULL) {
        if (curr_order->symbol_flag == 0) {
            curr_order = curr_order->next;
            continue;
        }

        /* First operand (symbol_name1) */
       if (curr_order->symbol_name1 != NULL) {
            symbol = search_symbol(symbol_head, curr_order->symbol_name1);
            if (symbol == NULL) {
                printf("Error: Undefined symbol %s\n", curr_order->symbol_name1);
                error_flag = 1;
                curr_order = curr_order->next;
                continue;
            }

            first_data_word = curr_order->word->next;
            if (symbol->type != 4) {
                /* Internal symbol: encode address in 8 bits + "10" for internal reference */
                strncpy(first_data_word->word, decode_number_in_8_bits(symbol->value), 8);
                strcpy(first_data_word->word + 8, "10\0");
            } else {
                /* External symbol: encode address in 8 bits + "01" for external reference */
                strncpy(first_data_word->word, decode_number_in_8_bits(symbol->value), 8);
                strcpy(first_data_word->word + 8, "01\0");
                printf("%s", first_data_word->word);

                /* Create external reference */
                new_ext = (Symbol*)safe_malloc(sizeof(Symbol), "Error: Memory allocation failed for extern symbol.");
                new_ext->name = (char*)safe_malloc(strlen(curr_order->symbol_name1) + 1, "Memory allocation failed for symbol name");
                strcpy(new_ext->name, curr_order->symbol_name1);
                new_ext->value = curr_order->IC + 1;  /* Address where symbol is used */
                new_ext->type = 4; /* Set type to external */
                new_ext->next = NULL;

                /* Add to external symbols list */
                if (*external_head == NULL) {
                    *external_head = new_ext;
                    extern_tail = new_ext;
                } else {
                    extern_tail->next = new_ext;
                    extern_tail = new_ext;
                }
            }
        }

        /* Second operand (symbol_name2) */
        if (curr_order->symbol_name2 != NULL) {
            symbol2 = search_symbol(symbol_head, curr_order->symbol_name2);
            if (symbol2 == NULL) {
                printf("Error: Undefined symbol %s\n", curr_order->symbol_name2);
                error_flag = 1;
                curr_order = curr_order->next;
                continue;
            }

            /* Find the placeholder "0000000000" */
            curr_word2 = curr_order->word;
            while (curr_word2 != NULL && strcmp(curr_word2->word, "0000000000\0") != 0) {
                curr_word2 = curr_word2->next;
            }

            if (curr_word2 == NULL) {
                printf("Error: Placeholder for second operand not found in instruction.\n");
                error_flag = 1;
                curr_order = curr_order->next;
                continue;
            }

            if (symbol2->type != 4) {
                strncpy(curr_word2->word, decode_number_in_8_bits(symbol2->value), 8);
                strcpy(curr_word2->word + 8, "10\0");
            }
            else {
                strncpy(curr_word2->word, decode_number_in_8_bits(symbol2->value), 8);
                strcpy(curr_word2->word + 8, "01\0");

                /* Create external reference */
                new_ext2 = (Symbol*)safe_malloc(sizeof(Symbol), "Error: Memory allocation failed for extern symbol.");
                new_ext2->name = (char*)safe_malloc(strlen(curr_order->symbol_name2) + 1, "Memory allocation failed for symbol name");
                strcpy(new_ext2->name, curr_order->symbol_name2);
                new_ext2->value = curr_word2->address;
                new_ext2->type = 4; /* Set type to external */
                new_ext2->next = NULL;

                /* Add to external symbols list */
                if (*external_head == NULL) {
                    *external_head = new_ext2;
                    extern_tail = new_ext2;
                } else {
                    extern_tail->next = new_ext2;
                    extern_tail = new_ext2;
                }
            }
        }

        curr_order = curr_order->next;
    }

    return error_flag;
}


/*
 * Frees all memory allocated for orders and their associated data.
 * Recursively destroys word lists and symbol names.
 */
void destroy_order(Order* head) {
    Order* current = head;
    while (current != NULL) {
        Order* next = current->next;

        if (current->word != NULL)
            destroy_word(current->word);
        if (current->symbol_name1 != NULL)
            free(current->symbol_name1);
        if (current->symbol_name2 != NULL)
            free(current->symbol_name2);


        current = next;
    }
}

/*
 * Adds a word to an order's word list.
 * Handles both empty list and non-empty list cases.
 * Sets appropriate addresses for each word.
 */
void add_word_to_order(Order* order, Word* word) {
    if (order->word == NULL) {
        order->word = word;
        order->word->address = order->IC;
    } else {
        Word* tmp = order->word;
        while (tmp->next != NULL) {
            tmp = tmp->next;
        }
        tmp->next = word;
        word->address = tmp->address + 1;
    }
    word->next = NULL;
}

