/*
 * decode.h - Binary encoding/decoding header
 * 
 * This header file declares functions for converting assembly language elements
 * to and from binary representations. It includes functions for:
 * - Number and character encoding
 * - Register and operand encoding
 * - Data section processing
 * - Base-4 encoding for output files
 */

#ifndef DECODE_H
#define DECODE_H

#include "order.h"
#include "word.h"

/* Binary encoding/decoding functions */
char* decode_number(int number);                                 /* 10-bit binary string from int */
char* decode_number_in_8_bits(int number);                      /* 8-bit binary string from int */
void decode_order_first_word(Order* order);                     /* Encodes first word of instruction */
char* decode_char(char ch);                                     /* 10-bit binary string from char */
char* reg_in_str(int num);                                      /* 4-bit binary string from register */
char* decode_source_register(int number);                       /* Source register encoding */
char* decode_target_register(int number);                       /* Target register encoding */
char* decode_registers(int number1,int number2);                /* Both registers encoding */
int str_to_int(const char* str);                                /* Converts string to int */
void decode_operand(Order* order,char* line,int index,int line_num); /* Decodes operand to binary */
int decode_data(Word **D_word_head, Word **D_word_tail, char *line, int index, int directive, int DC, int line_num); /* Decodes .data/.string/.mat */
char bin_to_base4char(const char *two_bits);                    /* Converts 2 bits to base-4 char */
char* binary_to_special_base4(const char *word);                /* Converts binary string to special base-4 */
char* address_to_base4(int address);                            /* Converts address to base-4 string */
char* header_address_to_base4(int address);                     /* Header address in base-4 */
char* header_code_to_base4(int code);                           /* Header code in base-4 */
int validate_operands(Order* order, int line_num);              /* Validates instruction operands */

#endif /* DECODE_H */
