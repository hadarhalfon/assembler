/*
 * main.c - Main assembler program
 * 
 * This file contains the main entry point and core assembly logic for a two-pass assembler.
 * It handles the first and second passes of assembly, managing symbol tables, orders,
 * and data words. The assembler processes assembly files and generates object files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"
#include "input.h"
#include "order.h"
#include "decode.h"
#include "symbolTable.h"
#include "macros.h"
#include "output.h"

/* Global variables for symbol tables and data structures */
Symbol *symbol_head = NULL;    /* Head of main symbol table */
Symbol *symbol_tail = NULL;    /* Tail of main symbol table */
Order *order_head = NULL;      /* Head of order list */
Order *order_tail = NULL;      /* Tail of order list */
Word *D_word_head = NULL;      /* Head of data word list */
Word *D_word_tail = NULL;      /* Tail of data word list */
Symbol *external_head = NULL;  /* Head of external symbol table */
int entries_flag = 0;          /* Flag for entry symbols */
int IC = 100;                  /* Instruction Counter (starts at 100) */
int DC = 0;                    /* Data Counter (starts at 0) */
int files;                     /* Number of files processed */

/*
 * This function cleans up all dynamically allocated memory including
 * symbol tables, order lists, and word lists. It resets all global
 * variables to their initial state.
 */
void end_system(void) {
    /* Free symbol tables */
    if (symbol_head != NULL) {
        destroy_symbol_table(symbol_head);
        symbol_head = NULL;
        symbol_tail = NULL;
    }

    if (external_head != NULL) {
        destroy_symbol_table(external_head);
        external_head = NULL;
    }

    /* Free order lists */
    if (order_head != NULL) {
        destroy_order(order_head);
        order_head = NULL;
        order_tail = NULL;
    }

    /* Free word lists */
    if (D_word_head != NULL) {
        destroy_word(D_word_head);
        D_word_head = NULL;
        D_word_tail = NULL;
    }

    /* Reset counters */
    IC = 100;
    DC = 0;
}

/*
 * This function reads the assembly file line by line and:
 * - Processes symbol definitions and adds them to symbol table
 * - Handles directives (.data, .string, .mat, .entry, .extern)
 * - Processes instructions and calculates their sizes
 * - Updates instruction and data counters
 */
int first_scan(FILE* file) {
    int directive;              /* Flag for directive processing */
    int symbol = 0;             /* Flag for symbol definition */
    int op = -1;                /* Opcode value */
    int num_of_ops = 0;         /* Number of operands */
    int index = 0;              /* Current position in line */
    int index2 = 0;             /* Temporary position marker */
    int index3 = 0;             /* Temporary position marker for validation */
    int L = 0;                  /* Label length */
    int num_of_line = 0;        /* Current line number */
    int num_of_chars = 0;       /* Number of characters in line */
    int error_flag = 0;         /* Error flag */
    int result = 0;             /* Result from decode_data function */
    Word* tmp = NULL;           /* Temporary word pointer */
    char *symbol1 = NULL;       /* Symbol name */
    char* line = NULL;          /* Current line */
    char* buffer = (char*)safe_malloc(sizeof(char)*MAX_LINE_LENGTH, "Out of memory");

    while (1) {
        num_of_chars = read_line(file, buffer);
        if (!num_of_chars) {
            break;
        }
        if (num_of_chars == 2) {
            printf("Error: Line too long (max 80 characters allowed)\n");
            error_flag = 1;
        }
        line = buffer; /* Use buffer directly instead of strcpy */
        num_of_line++;
        index = 0; /* Reset index for each line */
        
        /* Skip leading whitespace */
        while(line[index] == ' ')
            index++; /* index = location of symbol beginning */
            
        if (line[index] == '\n' || line[index] == '\r' || line[index] == '\0') { /* empty sentence */
            continue;
        }
        if (line[index] == ';') {
            continue; /* comment */
        }
        
        /* Check for symbol definition */
        index2 = is_symbol_definition(line, index); /* index2= location of ':' */
        if (index2 > 0) { /* Check if we found a symbol definition */
            symbol1 = (char*)safe_malloc(sizeof(char)*(index2-index+1), "Out of memory");
            strncpy(symbol1, line+index, index2-index);
            symbol1[index2-index] = '\0';
            symbol = 1;
            index = space_skip(line, index2+1); /* Skip past the colon and any spaces */
        }

        directive = is_directive(line, index);

        if (directive > 0 && directive < 4) { /* it is data or string or mat sentence */
            if(symbol) {
                if (search_symbol(symbol_head, symbol1) != NULL) {
                    printf("Error: Symbol already exists at line %d\n", num_of_line);
                    error_flag = 1;
                    free(symbol1);
                    symbol1 = NULL;
                    symbol = 0;
                    continue;
                }
                add_symbol(&symbol_head, &symbol_tail, symbol1);
                set_type(symbol_tail, 1); /* .data */
                set_value(symbol_tail, DC);
                symbol = 0;
                free(symbol1);
                symbol1 = NULL;
            }

            result = decode_data(&D_word_head, &D_word_tail, line, index, directive, DC, num_of_line);
            if (result == -1) {
                error_flag = 1;
                continue; /* Skip this line entirely if directive failed */
            }
            DC = result;
            continue;
        }
        
        if (directive == 5) { /* entry directive */
            if (symbol) {
                printf("Warning: in line %d symbol before entry\n", num_of_line);
                symbol = 0;
                free(symbol1);
                symbol1 = NULL;
            }
            index = space_skip(line, index+6); /* now index points to the symbol after the entry directive */
            index2 = is_symbol(line, index);
            if (index2 <= 0) { /* Check if we found a symbol */
                printf("Error: Expecting symbol after entry directive at line %d\n", num_of_line);
                error_flag = 1;
                continue; /* Skip this line entirely if directive failed */
            }
            continue; /* Only continue if directive was processed successfully */
        }
        
        if (directive == 4) { /* extern directive */
            if (symbol) {
                printf("Warning: in line %d symbol before extern.\n", num_of_line);
                symbol = 0;
                free(symbol1);
                symbol1 = NULL;
            }
            index = space_skip(line, index+7); /* now index points to the symbol after the extern directive */
            index2 = is_symbol(line, index);
            if (index2 <= 0) { /* Check if we found a symbol */
                printf("Error: Expecting symbol after extern directive at line %d\n", num_of_line);
                error_flag = 1;
                continue; /* Skip this line entirely if directive failed */
            }
            symbol1 = (char*)safe_malloc(sizeof(char)*(index2-index+1), "Out of memory");
            strncpy(symbol1, line+index, index2-index);
            symbol1[index2-index] = '\0';
            add_symbol(&symbol_head, &symbol_tail, symbol1);
            set_type(symbol_tail, 4); /* .extern */
            free(symbol1);
            symbol1 = NULL;
            continue; /* Only continue if directive was processed successfully */
        }
        
        if (symbol) { /* an order sentence */
            if (search_symbol(symbol_head, symbol1) != NULL) {
                printf("Error: Symbol already exists at line %d\n", num_of_line);
                error_flag = 1;
                free(symbol1);
                symbol1 = NULL;
                symbol = 0;
                continue;
            }
            symbol_tail = add_symbol(&symbol_head, &symbol_tail, symbol1);
            set_type(symbol_tail, 2);
            set_value(symbol_tail, IC);
            symbol = 0;
            free(symbol1);
            symbol1 = NULL;
        }

        op = opcode_in_decimal(line, index, num_of_line);
        if (op == -1) {
            printf("Error: Invalid function at line %d\n", num_of_line);
            error_flag = 1;
            continue;
        }
        num_of_ops = number_of_operands(op);
        add_order(&order_head, &order_tail, op);
        order_tail->IC = IC;
        order_tail->word = NULL;
        
        if (num_of_ops == 0) {
            if (op == 15) {
                if (space_skip(line, index+4) != -2) { /* after stop we expect to see space or '\n', no operands */
                    printf("Error: Function 'stop' expects no arguments at line %d\n", num_of_line);
                    error_flag = 1;
                    continue;
                }
            }
            else if (space_skip(line, index+3) != -1) { /* after rts we expect to see space or '\n' , no arguments. */
                printf("Error: Function 'rts' expects no arguments at line %d\n", num_of_line);
                error_flag = 1;
                continue;
            }
            decode_order_first_word(order_tail);
            order_tail->operand1 = -1;
            order_tail->operand2 = -1;
            order_tail->number_of_words = 1; /* we need to check if I missed memory allocation */
            
            /* Validate operands for instructions with no operands */
            if (validate_operands(order_tail, num_of_line)) {
                error_flag = 1;
                continue;
            }
            
            IC += 1;
            continue;
        }
        
        index += 3; /* now index is the location after the function name */
        index = space_skip(line, index); /* now index is the location of the operand */
        
        if (num_of_ops == 1) {
            order_tail->operand2 = addressing_method(line, index, num_of_line);
            decode_order_first_word(order_tail);
            order_tail->operand1 = -1;
            
            /* Check for extra characters after the operand */
            index2 = index;
            while (index2 < strlen(line) && line[index2] != ' ' && line[index2] != '\t' && line[index2] != '\n' && line[index2] != '\r' && line[index2] != '\0') {
                index2++;
            }
            /* Skip spaces after operand */
            while (index2 < strlen(line) && (line[index2] == ' ' || line[index2] == '\t')) {
                index2++;
            }
            /* Check if there are any non-space characters after the operand */
            if (index2 < strlen(line) && line[index2] != '\n' && line[index2] != '\r' && line[index2] != '\0') {
                printf("Error: Extra characters after operand at line %d\n", num_of_line);
                error_flag = 1;
                continue;
            }
            
            L = number_of_lines(order_tail->operand1, order_tail->operand2);
            order_tail->number_of_words = L;
            order_tail->IC = IC;
            decode_operand(order_tail, line, index, num_of_line);
            
            /* Validate operands for single-operand instructions */
            if (validate_operands(order_tail, num_of_line)) {
                error_flag = 1;
                continue;
            }
            
            IC += L;
            continue;
        }
        
        if (num_of_ops == 2) {
            order_tail->operand1 = addressing_method(line, index, num_of_line);

            index2 = index;
            while (index2 < strlen(line) && line[index2] != ',' && line[index2] != '\n' && line[index2] != '\0') {
                index2++;
            }
            if (index2 >= strlen(line) || line[index2] != ',') {
                printf("Error: Missing comma between operands at line %d\n", num_of_line);
                error_flag = 1;
                continue;
            }
            index2++;
            index2 = space_skip(line, index2); /* now index 2 is the location of the second operand */
            order_tail->operand2 = addressing_method(line, index2, num_of_line);

            /* Check for extra characters after the second operand */
            index3 = index2;
            while (index3 < strlen(line) && line[index3] != ' ' && line[index3] != '\t' && line[index3] != '\n' && line[index3] != '\r' && line[index3] != '\0') {
                index3++;
            }
            /* Skip spaces after second operand */
            while (index3 < strlen(line) && (line[index3] == ' ' || line[index3] == '\t')) {
                index3++;
            }
            /* Check if there are any non-space characters after the second operand */
            if (index3 < strlen(line) && line[index3] != '\n' && line[index3] != '\r' && line[index3] != '\0') {
                printf("Error: Extra characters after second operand at line %d\n", num_of_line);
                error_flag = 1;
                continue;
            }

            L = number_of_lines(order_tail->operand1, order_tail->operand2);
            order_tail->number_of_words = L;
            order_tail->IC = IC;
            decode_order_first_word(order_tail);
            
            if (order_tail->operand1 == 3 && order_tail->operand2 == 3) {
                tmp = new_word();
                strcpy(tmp->word, decode_registers(atoi(line+index+1), atoi(line+index2+1)));
                add_word_to_order(order_tail, tmp);
            }
            else {
                decode_operand(order_tail, line, index, num_of_line);
                decode_operand(order_tail, line, index2, num_of_line);
            }
            
            /* Validate operands for two-operand instructions */
            if (validate_operands(order_tail, num_of_line)) {
                error_flag = 1;
                continue;
            }
            
            IC += L;
        }
    }
    
    update_data_symbols_value(symbol_head, IC);
    free(buffer); /* Free the allocated buffer */
    return error_flag;
}


/*
 * This function performs the second pass which:
 * - Processes .entry directives and validates entry symbols
 * - Updates symbol operands with final addresses
 * - Resolves external symbol references
 * - Validates symbol existence and types
 */
int second_scan(FILE* file) {
    int directive;              /* Directive type */
    int index;                  /* Current position in line */
    int index2;                 /* Temporary position marker */
    int error_flag = 0;         /* Error flag */
    int num_of_line = 0;        /* Current line number */
    int num_of_chars = 0;
    int update_result = 0;      /* Result from update_symbol_operands */
    char *symbol1 = NULL;       /* Symbol name */
    char* line = NULL;          /* Current line */
    char* buffer = (char*)safe_malloc(sizeof(char)*MAX_LINE_LENGTH, "Out of memory");
    
    
    while (1) {
        num_of_chars = read_line(file, buffer);
        if (!num_of_chars) {
            break;
        }
        if (num_of_chars == 2) {
            printf("Error: Line too long (max 80 characters allowed)\n");
            error_flag = 1;
        } 
        line = buffer; /* Use buffer directly instead of strcpy */
        num_of_line++;
        index = 0; /* Reset index for each line */
        index = space_skip(line, index); /* index = location of symbol beginning */
        
        if (line[index] == '\n' || line[index] == '\r' || line[index] == '\0') /* empty sentence */
            continue;
        if (line[index] == ';') /* comment */
            continue;
            
        index2 = is_symbol_definition(line, index);
        if (index2) {
            index = index2+1; /* +1 for the ':' */
        }
        directive = is_directive(line, index);
        if (directive > 0 && directive < 5) {
            continue;
        }
        if (directive == 5) {
            entries_flag = 1;
            index += 6;
            index = space_skip(line, index);
            index2 = is_symbol(line, index);
            if (!index2) {
                printf("Error: Entry directive expects exactly one symbol argument at line %d\n", num_of_line);
                error_flag = 1;
                continue;
            }
            symbol1 = (char*)safe_malloc(sizeof(char)*MAX_SYMBOL_LENGTH, "Out of memory");
            strncpy(symbol1, line+index, index2-index+1);
            symbol1[index2-index] = '\0';
            if (search_symbol(symbol_head, symbol1) == NULL) {
                printf("Error: Symbol does not exist at line %d\n", num_of_line);
                error_flag = 1;
                free(symbol1);
                symbol1 = NULL;
                continue;
            }

            set_type(search_symbol(symbol_head, symbol1), 3); /* we need to check that type 3 is entry */
            free(symbol1);
            symbol1 = NULL;
        }
    }
    
    update_result = update_symbol_operands(order_head, symbol_head, &external_head); /* in the update function we should make a list of the external symbols */
    if (update_result) {
        error_flag = 1;
    }
    free(buffer); /* Free the allocated buffer */
    return error_flag; /* if error flag is on we should return 0 */
}

/*
 * This function orchestrates the two-pass assembly process:
 * 1. Preprocessor phase (macro expansion)
 * 2. First pass (symbol collection and size calculation)
 * 3. Second pass (address resolution and validation)
 * 4. Output generation (.ob, .ent, .ext files)
 */
int main(int argc,char *argv[]) {
    FILE *file = NULL;          /* Input file pointer */
    FILE *fp1 = NULL;           /* Preprocessed file pointer */
    char *fullfilename = NULL;  /* Full filename with path */
    char *filename = NULL;      /* Base filename */
    int files;                  /* File counter */


    if (argc < 2) {
        printf("No files were send to the assembler\n");
        exit(0);
    }

    for (files = 1 ; files < argc ; files++) {

        filename = argv[files];
        if (!filename) {
            printf("Error: invalid filename\n");
            exit(0);
        }

        /* Allocate memory for fullfilename */
        fullfilename = create_filename(filename, ".as");

        file = fopen(fullfilename,"r");
        if (!file) {
            printf("Can't open file %s or it does not exist.\n",fullfilename);
            free(fullfilename);
            continue;
        }
        printf("Processing file: %s\n", fullfilename);

        fp1 = preprocessor(file,filename);
        
        if (fp1 == NULL) {
            printf("Can't finish the assembler process on file: %s.\n", fullfilename);
            fclose(file);
            free(fullfilename);
            continue;
        }

        if (first_scan(fp1)){
            printf("Can't finish the assembler process on file: %s.\n", fullfilename);
            fclose(file);
            free(fullfilename);
            end_system();
            continue;
        }
        rewind(fp1);

        /* fopen(fullfilename,"r"); */
        if (second_scan(fp1)){
            printf("Can't finish the assembler process on file: %s.\n", fullfilename);
            fclose(file);
            free(fullfilename);
            end_system();
            continue;
        }
        
        /* Only create output files if both scans completed successfully */
        update_data(D_word_head,IC);

        if (external_head != NULL) {
            build_ext(external_head,filename);
        }
        if (entries_flag) {
            build_ent(symbol_head,filename);
        }
        build_ob(D_word_head,order_head,filename,IC-100,DC);

        /* Clean up */
        fclose(file);
        free(fullfilename);
        end_system();
    }

    return 0;
}