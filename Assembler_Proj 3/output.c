/*
 * output.c - Output file generation
 * 
 * This file contains functions for generating the three output files
 * produced by the assembler: .ob (object), .ent (entry), and .ext (external).
 * It handles:
 * - Object file generation with binary data
 * - Entry symbol file generation
 * - External symbol file generation
 * - Base-4 encoding for addresses and data
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decode.h"
#include "word.h"
#include "symbolTable.h"
#include "helpers.h"

/*
 * Creates a .ext file containing all external symbols and their addresses.
 * External symbols are those declared with .extern directive and used
 * in the assembly code. The addresses are encoded in base-4 format.
 */
void build_ext(Symbol* external_head,char *filename) {
    FILE* file;
    char *fullfilename = create_filename(filename, ".ext");
    Symbol* current_head;
    file = fopen(fullfilename,"w");
    if (!file) {
        printf("Error: Could not create file: %s\n",fullfilename);
        free(fullfilename);
        exit(1);
    }
    current_head = external_head;
    while (current_head != NULL) {
         /* extern type */
        fprintf(file,"%s\t%s\n",current_head->name,address_to_base4(current_head->value));
        current_head = current_head->next;
    }
    fclose(file);
    free(fullfilename);
}

/*
 * Creates a .ent file containing all entry symbols and their addresses.
 * Entry symbols are those declared with .entry directive. The addresses
 * are encoded in base-4 format.
 */
void build_ent(Symbol* symbol_head,char *filename) {
    FILE* file;
    char *fullfilename = create_filename(filename, ".ent");
    Symbol* current_head;
    file = fopen(fullfilename,"w");
    if (!file) {
        printf("Error: Could not create file %s\n",fullfilename);
        free(fullfilename);
        exit(1);
    }
    current_head = symbol_head;
    while (current_head != NULL) {
        if (current_head->type == 3) {  /* Entry symbol type */
            fprintf(file,"%s\t%s\n",current_head->name,address_to_base4(current_head->value));
        }
        current_head = current_head->next;
    }
    fclose(file);
    free(fullfilename);
}

/*
 * Creates a .ob file containing the assembled program data. The file includes:
 * - Header with instruction and data counters
 * - All instruction words with their addresses
 * - All data words with their addresses
 * - All addresses and data encoded in base-4 format
 */
void build_ob(Word* Dword_head,Order* order_head, char *filename,int ICF,int DCF) {
    FILE* file;
    char *fullfilename = create_filename(filename, ".ob");
    Order* current_order;
    Word* current_data_word;
    file = fopen(fullfilename,"w");
    if (!file) {
        printf("Error: Could not create file %s\n",fullfilename);
        free(fullfilename);
        exit(1);
    }
    /* Write header with instruction and data counters */
    fprintf(file,"\t%s\t%s\n",header_address_to_base4(ICF),header_code_to_base4(DCF));

    /* Process order words (instructions) */
    current_order = order_head;
    while (current_order != NULL) {
        Word* current_word = current_order->word;
        while (current_word != NULL) {
            fprintf(file,"%s\t%s\n",address_to_base4(current_word->address),binary_to_special_base4(current_word->word));
            current_word = current_word->next;
        }
        current_order = current_order->next;
    }

    /* Process data words */
    current_data_word = Dword_head;
    while (current_data_word != NULL) {
        fprintf(file,"%s\t%s\n",address_to_base4(current_data_word->address),binary_to_special_base4(current_data_word->word));
        current_data_word = current_data_word->next;
    }
    fclose(file);
    free(fullfilename);
}