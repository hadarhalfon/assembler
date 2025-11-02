/*
 * decode.c - Binary encoding and decoding functions
 * 
 * This file contains functions for converting assembly language elements
 * into binary representations and vice versa. It handles:
 * - Number encoding/decoding in various bit lengths
 * - Character encoding in binary format
 * - Register encoding for source and target operands
 * - Operand decoding and validation
 * - Data section processing (.data, .string, .mat)
 * - Base-4 encoding for output files
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"
#include "order.h"
#include "word.h"

/*
 * This function implements comprehensive operand validation for all assembly instructions.
 * It checks that operands follow the correct addressing mode rules for each instruction type.
 */
int validate_operands(Order* order, int line_num) {
    int opcode = order->opcode;
    int operand1 = order->operand1;
    int operand2 = order->operand2;
    
    /* Handle instructions with no operands */
    if (opcode == 14 || opcode == 15) { /* rts, stop */
        if (operand1 != -1 || operand2 != -1) {
            printf("Error: Instruction '%s' expects no operands at line %d\n", 
                   (opcode == 14) ? "rts" : "stop", line_num);
            return 1;
        }
        return 0;
    }
    
    /* Handle two-operand instructions */
    if (opcode >= 0 && opcode <= 3) { /* mov, cmp, add, sub */
        /* Source operand validation */
        if (operand1 < 0 || operand1 > 3) {
            printf("Error: Invalid source operand type at line %d\n", line_num);
            return 1;
        }
        
        /* Destination operand validation */
        if (operand2 < 0 || operand2 > 3) {
            printf("Error: Invalid destination operand type at line %d\n", line_num);
            return 1;
        }
        
        /* For mov, add, sub: destination cannot be immediate (0) or matrix (2) */
        if (opcode == 0 || opcode == 2 || opcode == 3) {
            if (operand2 == 0) {
                printf("Error: Destination operand cannot be immediate at line %d\n", line_num);
                return 1;
            }
            if (operand2 == 2) {
                printf("Error: Destination operand cannot be matrix at line %d\n", line_num);
                return 1;
            }
        }
        
        return 0;
    }
    
    /* Handle lea instruction */
    if (opcode == 4) { /* lea */
        /* Source must be symbol or matrix */
        if (operand1 != 1 && operand1 != 2) {
            printf("Error: LEA source operand must be a label at line %d\n", line_num);
            return 1;
        }
        
        /* Destination allowed types: 1 (symbol), 2 (matrix), 3 (register) */
        if (operand2 != 1 && operand2 != 2 && operand2 != 3) {
            printf("Error: LEA destination operand must be label or register at line %d\n", line_num);
            return 1;
        }
        
        return 0;
    }
    
    if (opcode > 4 && opcode < 13) { 
        /* Should have only one operand */
        if (operand1 != -1) {
            printf("Error: Unary instruction '%s' expects only one operand at line %d\n", 
                   (opcode == 4) ? "not" : (opcode == 5) ? "clr" : (opcode == 7) ? "inc" : "dec", line_num);
            return 1;
        }
        
        /* Allowed types: symbol (1), matrix (2), register (3) */
        if (operand2 != 1 && operand2 != 2 && operand2 != 3 ) {
            printf("Error: Unary instruction operand must be label or register at line %d\n", line_num);
            return 1;
        }
        
        return 0;
    }

    
    /* Handle prn instruction */
    if (opcode == 13) { /* prn */
        /* Should have only one operand */
        if (operand1 != -1) {
            printf("Error: PRN instruction expects only one operand at line %d\n", line_num);
            return 1;
        }
        
        /* All operand types allowed (0, 1, 2, 3) */
        if (operand2 < 0 || operand2 > 3) {
            printf("Error: Invalid PRN operand type at line %d\n", line_num);
            return 1;
        }
        
        return 0;
    }
    
    /* Default case: invalid opcode */
    return 1;
}
    
/*
 * Converts a signed integer to its 10-bit binary representation.
 * Uses a mask to ensure only the lowest 10 bits are considered.
 */
char* decode_number(int number) {
    char* str = (char*) safe_malloc(sizeof(char) * 11, "Memory allocation failed in decode_number");
    unsigned int mask = (1 << 10) - 1;  /* = 0x3FF = 1023 */
    unsigned int val = number & mask;
    int i;
    for (i=9 ; i>=0; i--) {
        str[9 - i] = ((val >> i) & 1) ? '1' : '0';
    }
    str[10] = '\0';
    return str;
}

/*
 * Converts a signed integer to its 8-bit binary representation.
 * Handles negative numbers using two's complement representation.
 */
char* decode_number_in_8_bits(int number) {
    char* str = (char*) safe_malloc(sizeof(char) * 9, "Memory allocation failed in decode_number_in_8_bits");

    unsigned int val;
    int i;

    if (number < 0)
        val = (1 << 8) + number;  /* Two's complement for 8 bits */
    else
        val = number;

    for (i = 7; i >= 0; i--) {
        str[7 - i] = ((val >> i) & 1) ? '1' : '0';
    }
    str[8] = '\0';
    return str;
}

/*
 * Constructs the first word of an assembly instruction by combining:
 * - Opcode (bits 2-5)
 * - Source operand addressing mode (bits 6-7)
 * - Destination operand addressing mode (bits 8-9)
 * - Reserved bits (bits 0-1)
 */
void decode_order_first_word(Order* order) {
    Word* tmp = (Word*) safe_malloc(sizeof(Word), "Memory allocation failed in decode_order_first_word");

    char* str = (char*) safe_malloc(11, "Memory allocation failed for str in decode_order_first_word");

    strncpy(str, decode_number(order->opcode) + 6, 4); /* bits 2-5 */
    strncpy(str + 4, decode_number(order->operand1) + 8, 2); /* bits 6-7 */
    strncpy(str + 6, decode_number(order->operand2) + 8, 2); /* bits 8-9 */
    strcpy(str + 8, "00\0"); /* bits 0-1 */
    tmp->word = (char*) safe_malloc(sizeof(char) * 11, "Memory allocation failed for word field");
    strncpy(tmp->word, str, 11);
    tmp->type = 1;
    tmp->next = NULL;

    add_word_to_order(order, tmp);
    free(str);
}

/*
 * Converts a character to its 10-bit binary representation.
 * The character is treated as an unsigned value (0-255).
 */
char* decode_char(char ch) {
    char *bin = (char *)safe_malloc(11, "Memory allocation failed in decode_char");
    unsigned int val;
    int i;

    val = (unsigned char)ch;  /* force into range 0–255 */

    for (i = 9; i >= 0; i--) {
        bin[9 - i] = ((val >> i) & 1) ? '1' : '0';
    }

    bin[10] = '\0';
    return bin;
}

/*
 * Converts a register number (0-15) to its 4-bit binary representation.
 */
char* reg_in_str(int num) {
    char* str = (char*) safe_malloc(sizeof(char) * 5, "Memory allocation failed in reg_in_str");
    int i;
    for (i = 3; i >= 0; i--) {
        str[3 - i] = ((num >> i) & 1) ? '1' : '0';
    }
    str[4] = '\0';
    return str;
}

/*
 * Creates the binary encoding for a target register operand.
 * Format: "0000" + register_bits + "00"
 */
char* decode_target_register(int number) {
    char* str = (char*) safe_malloc(sizeof(char) * 11, "Memory allocation failed in decode_target_register");
    strcpy(str,"0000");
    strncpy(str+4,reg_in_str(number),4);
    strncpy(str+8,"00\0",3);
    return str;
}

/*
 * Creates the binary encoding for a source register operand.
 * Format: "00" + register_bits + "0000"
 */
char* decode_source_register(int number) {
    char* str = (char*) safe_malloc(sizeof(char) * 11, "Memory allocation failed in decode_source_register");
    strncpy(str,reg_in_str(number),4);
    strcpy(str+4,"000000\0");
    return str;
}

/*
 * Creates the binary encoding for a matrix operand.
 * Format: register1_bits + register2_bits + "00"
 */
char* decode_registers(int number1,int number2) {
    char* str = (char*) safe_malloc(sizeof(char) * 11, "Memory allocation failed in decode_registers");
    strncpy(str,reg_in_str(number1),4);
    strncpy(str+4,reg_in_str(number2),4);
    strncpy(str+8,"00\0",3);
    return str;
}

/*
 * Converts a string to an integer.
 * Handles positive and negative numbers.
 */
int str_to_int(const char* str) {
    int result = 0, sign = 1;
    int index = 0;

    if (str[index] == '-') {
        sign = -1;
        index++;
    } else if (str[index] == '+') {
        index++;
    }

    while (str[index] >= '0' && str[index] <= '9') {
        result = result * 10 + (str[index] - '0');
        index++;
    }

    return result * sign;
}

/*
 * Decodes an operand from a line of assembly code.
 * Handles immediate, register, and symbol operands.
 * Validates matrix operand syntax.
 * Updates order structure with operand information.
 */
void decode_operand(Order* order,char* line,int index,int line_num) { /* we need to check if there is memory allocation to free and add checks that the allocation succeed (not null) */
    char* str = (char*) safe_malloc(sizeof(char) * 11, "Memory allocation failed in decode_operand");
    char* symbol_name = (char*) safe_malloc(sizeof(char) * MAX_SYMBOL_LENGTH, "Memory allocation failed for symbol_name");
    Word* tmp = NULL;
    Word* tmp2 = NULL;
    int num, index2;
    int flag = 0;


    if (str == NULL) {
        printf("Error in decode_operand() at line %d - str allocation failed\n", line_num);
        exit(1);
    }

    tmp = new_word();
    tmp2 = new_word();

    if (tmp == NULL || tmp2 == NULL) {
        printf("Error in decode_operand() at line %d - word allocation failed\n", line_num);
        free(str);
        free(symbol_name);
        exit(1);
    }

    if (line == NULL) {
        printf("Error in decode_operand() at line %d - line is NULL\n", line_num);
        free(str);
        free(symbol_name);
        free(tmp);
        free(tmp2);
        exit(1);
    }
    if (index < 0 || index >= strlen(line)) {
        printf("Error in decode_operand() at line %d - index %d out of bounds (line length: %d)\n", line_num, index, strlen(line));
        free(str);
        free(symbol_name);
        free(tmp);
        free(tmp2);
        exit(1);
    }

    /* Check for immediate addressing (#number) */
    if (line[index] == '#') {
        index++;
        if (is_number(line, index)) {
            num = str_to_int(line + index);
            strncpy(str, decode_number_in_8_bits(num), 8);  /* 8-bit immediate value */
            strncpy(str+8,"00\0",3);                        /* Addressing mode bits */
        } else {
            printf("Error in decode_operand() at line %d - invalid immediate value\n", line_num);
            free(str);
            free(symbol_name);
            free(tmp);
            free(tmp2);
            exit(1);
        }
    }
    else if ( is_register(line,index,line_num) == 1 ) {
        free(str); /* Free the original allocation */
        str = decode_source_register(atoi(line+index+1));  /* Extract register number after 'r' */
    }
    else if (is_register(line,index,line_num) == 2) {
        free(str); /* Free the original allocation */
        str = decode_target_register(atoi(line+index+1));  /* Extract register number after 'r' */
    }
    else {
        index2 = is_symbol(line,index);
        if (index2 <= 0) {
            printf("Error in decode_operand() at line %d - invalid symbol\n", line_num);
            free(str);
            free(symbol_name);
            free(tmp);
            free(tmp2);
            exit(1);
        }
        
        /* Set symbol flag and copy symbol name */
        order->symbol_flag = 1;
        strncpy(symbol_name,line+index,index2-index);
        symbol_name[index2-index] = '\0'; /* Ensure null termination */


        /* Check if this symbol is part of a matrix operand */
        if (is_mat_operand(line,index,line_num)) {
            int reg_index, reg1_start, reg2_start, reg1_len, reg2_len, i;
            char reg1_str[10], reg2_str[10];
            /* Process matrix operand - find the register indices */

            reg_index = index2; /* Start after the symbol */
            while (reg_index < strlen(line) && line[reg_index] != '[') {
                reg_index++;
            }
            if (line[reg_index] == '[') {
                reg_index++; /* Skip the '[' */
                reg1_start = reg_index;
                while (reg_index < strlen(line) && line[reg_index] != ']') {
                    reg_index++;
                }
                if (line[reg_index] == ']') {
                    reg_index++; /* Skip the ']' */
                    if (line[reg_index] == '[') {
                        reg_index++; /* Skip the second '[' */
                        reg2_start = reg_index;
                        while (reg_index < strlen(line) && line[reg_index] != ']') {
                            reg_index++;
                        }
                        if (line[reg_index] == ']') {
                            /* Extract register numbers */
                            reg1_len = 0;
                            reg2_len = 0;

                            /* Extract first register */
                            i = reg1_start;
                            while (i < strlen(line) && line[i] != ']' && reg1_len < 9) {
                                if (line[i] == 'r') {
                                    i++;
                                    while (i < strlen(line) && isdigit(line[i]) && reg1_len < 9) {
                                        reg1_str[reg1_len++] = line[i];
                                        i++;
                                    }
                                } else {
                                    i++;
                                }
                            }
                            reg1_str[reg1_len] = '\0';

                            /* Extract second register */
                            i = reg2_start;
                            while (i < strlen(line) && line[i] != ']' && reg2_len < 9) {
                                if (line[i] == 'r') {
                                    i++;
                                    while (i < strlen(line) && isdigit(line[i]) && reg2_len < 9) {
                                        reg2_str[reg2_len++] = line[i];
                                        i++;
                                    }
                                } else {
                                    i++;
                                }
                            }
                            reg2_str[reg2_len] = '\0';


                            /* Decode the registers */
                            strcpy(tmp2->word, decode_registers(atoi(reg1_str), atoi(reg2_str)));
                            flag=1;
                        }
                    }
                }
            }
        }

        /* Allocate memory for symbol names if needed */
                    if (order->symbol_name1 == NULL && (order->operand1 == 2 || order->operand1 == 1)) {
                order->symbol_name1 = (char*)safe_malloc(strlen(symbol_name) + 1, "Memory allocation failed for symbol_name1");
                if (order->symbol_name1 == NULL) {
                    printf("Error in decode_operand() at line %d - memory allocation failed\n", line_num);
                    free(str);
                    free(symbol_name);
                    free(tmp);
                    free(tmp2);
                    exit(1);
                }
            strcpy(order->symbol_name1, symbol_name);
        }
        else {
            order->symbol_name2 = (char*)safe_malloc(strlen(symbol_name) + 1, "Memory allocation failed for symbol_name2");
            if (order->symbol_name2 == NULL) {
                printf("Error in decode_operand() at line %d - memory allocation failed\n", line_num);
                free(str);
                free(symbol_name);
                free(tmp);
                free(tmp2);
                exit(1);
            }
            strcpy(order->symbol_name2, symbol_name);
        }
        strcpy(str, "0000000000\0");
    }

    strcpy(tmp->word,str);

    add_word_to_order(order,tmp);
    free(str);

    if (flag)
        add_word_to_order(order,tmp2);
}

/*
 * Decodes a .data directive.
 * Handles data initialization and matrix initialization.
 * Validates data format and matrix syntax.
 * Updates DC (data counter) and adds words to the data list.
 */
int decode_data(Word **D_word_head, Word **D_word_tail, char *line, int index, int directive, int DC, int line_num) {
    int i;
    Word *tmp = NULL;

    if (directive == 1) { /* .data */
        index += 5;
        
        /* Check for comma errors first */
        if (contains_invalid_commas(line, index)) {
            int error_type = get_data_comma_error_type(line, index);
            switch (error_type) {
                case 1:
                    printf("Error: Leading comma in .data directive at line %d\n", line_num);
                    break;
                case 2:
                    printf("Error: Trailing comma in .data directive at line %d\n", line_num);
                    break;
                case 3:
                    printf("Error: Double comma in .data directive at line %d\n", line_num);
                    break;
                case 4:
                    printf("Error: Missing comma between values in .data directive at line %d\n", line_num);
                    break;
                default:
                    printf("Error: Invalid comma usage in .data directive at line %d\n", line_num);
                    break;
            }
            return -1;  /* Return error value */
        }        
        /* Validate data format */
        if (!is_legal_data_or_matrix_initialization(line, index, line_num)) {
            return -1;  /* Return error value */
        }

        while (line[index] != '\n' && line[index] != '\0') {
            index = space_skip(line, index);
            if (line[index] == '\n' || line[index] == '\0') break;

            if (!isdigit(line[index]) && line[index] != '-' && line[index] != '+') {
                printf("Error: Expected number at index %d but found '%c' at line %d\n", index, line[index], line_num);
                break;
            }

            tmp = add_word(D_word_head, D_word_tail);
            if (!tmp) {
                printf("Error in memory allocation\n");
                return DC;
            }


            tmp->address = DC;
            strncpy(tmp->word, decode_number(str_to_int(line + index)), 11);
            DC++;

            while (line[index] != ',' && line[index] != '\n' && line[index] != '\0') {
                index++;
            }

            if (line[index] == ',') {
                index++; /* skip comma */
            }
        }
    }
    else if (directive == 2) { /* .string */
        index += 7;
        index = space_skip(line, index);

        if (!is_legal_string(line, index, line_num)) {
            return -1;  /* Return error value */
        }

        index++; /* skip starting " */

        while (line[index] != '"' && line[index] != '\0') {
            tmp = add_word(D_word_head, D_word_tail);
            if (!tmp) {
                printf("Error in memory allocation\n");
                return DC;
            }

            strncpy(tmp->word, decode_char(line[index]), 11);
            tmp->address = DC;
            DC++;
            index++;
        }
        tmp = add_word(D_word_head, D_word_tail);
        if (!tmp) {
            printf("Error in memory allocation\n");
            return DC;
        }
        strncpy(tmp->word, "0000000000\0", 11);
        tmp->address = DC;
        DC++;
    }

    else if (directive == 3) { /* .mat */
        int dim_end;
        int data_start;
        int dim;
        
        index += 4;
        while (line[index] != '[' && line[index] != '\0')
            index++;

        /* Check for comma errors in matrix data initialization */
        /* First, find the end of the matrix dimensions */
        dim_end = index;
        for (dim = 0; dim < 2; dim++) {
            while (dim_end < strlen(line) && line[dim_end] != '[') dim_end++;
            if (line[dim_end] == '[') dim_end++;
            while (dim_end < strlen(line) && line[dim_end] != ']') dim_end++;
            if (line[dim_end] == ']') dim_end++;
        }
        
        /* Now find the start of the data initialization */
        data_start = space_skip(line, dim_end);
        
        if (data_start < strlen(line) && line[data_start] != '\n' && line[data_start] != '\0') {
            if (contains_invalid_commas(line, data_start)) {
                int error_type = get_data_comma_error_type(line, data_start);
                switch (error_type) {
                    case 1:
                        printf("Error: Leading comma in .mat directive at line %d\n", line_num);
                        break;
                    case 2:
                        printf("Error: Trailing comma in .mat directive at line %d\n", line_num);
                        break;
                    case 3:
                        printf("Error: Double comma in .mat directive at line %d\n", line_num);
                        break;
                    case 4:
                        printf("Error: Missing comma between values in .mat directive at line %d\n", line_num);
                        break;
                    default:
                        printf("Error: Invalid comma usage in .mat directive at line %d\n", line_num);
                        break;
                }
                return -1;  /* Return error value */
            }
        }

        if (!is_legal_mat(line, index, line_num)) {
            return -1;  /* Return error value */
        }

        for (i = 0; i < 2; i++) {
            while (line[index] != ']' && line[index] != '\0') index++;
            if (line[index] == ']') index++;
        }

        index = space_skip(line, index);

        if (line[index] == '\n' || line[index] == '\0') {
            int cells = save_place(line, line_num);
            if (cells != -1) DC += cells;
            return DC;
        }

        while (line[index] != '\n' && line[index] != '\0') {
            index = space_skip(line, index);
            if (!isdigit(line[index]) && line[index] != '-' && line[index] != '+') break;

            tmp = add_word(D_word_head, D_word_tail);
            if (!tmp) {
                printf("Error in memory allocation\n");
                return DC;
            }

            strncpy(tmp->word, decode_number(str_to_int(line + index)), 11);
            tmp->address = DC;
            DC++;

            while (line[index] != ',' && line[index] != '\n' && line[index] != '\0') {
                index++;
            }

            if (line[index] == ',') {
                index++;
            }
        }
    }

    return DC;
}

/*
 * Converts a 2-bit binary string to a base 4 character.
 * Validates the input string and returns the corresponding base 4 character.
 */
char bin_to_base4char(const char *two_bits) {
    if (strncmp(two_bits, "00", 2) == 0) return 'a';
    if (strncmp(two_bits, "01", 2) == 0) return 'b';
    if (strncmp(two_bits, "10", 2) == 0) return 'c';
    if (strncmp(two_bits, "11", 2) == 0) return 'd';
    return '?'; /* invalid input */
}

/*
 * Converts a 5-bit binary string to a special base 4 string.
 * Validates the input string and returns the corresponding base 4 string.
 */
char* binary_to_special_base4(const char *word) {
    char *result = (char *)safe_malloc(6, "Memory allocation failed in binary_to_special_base4");  /* 5 chars + null terminator */
    int i;
    if (!result) return NULL;

    for (i = 0; i < 5; i++) {
        result[i] = bin_to_base4char(word + (i * 2));
    }
    result[5] = '\0';
    return result;
}

/*
 * Converts an address to a base 4 string.
 * Validates the input address and returns the corresponding base 4 string.
 */
char* address_to_base4(int address) {
    char *result = (char *)safe_malloc(5, "Memory allocation failed in address_to_base4");  /* 4 chars + null terminator */
    int i;
    if (!result) return NULL;

    /* Convert address to base 4 (4 characters) */
    for (i = 0; i < 4; i++) {
        int digit = (address >> (6 - i * 2)) & 3;  /* Extract 2 bits at position 6-i*2 */
        switch (digit) {
            case 0: result[i] = 'a'; break;  /* 00 -> 'a' */
            case 1: result[i] = 'b'; break;  /* 01 -> 'b' */
            case 2: result[i] = 'c'; break;  /* 10 -> 'c' */
            case 3: result[i] = 'd'; break;  /* 11 -> 'd' */
        }
    }
    result[4] = '\0';
    return result;
}

/*
 * Converts a header address to a base 4 string.
 * Validates the input address and returns the corresponding base 4 string.
 */
char* header_address_to_base4(int address) {
    char *result = (char *)safe_malloc(4, "Memory allocation failed in header_address_to_base4");  /* 3 chars + null terminator */
    int i;
    if (!result) return NULL;

    /* Convert address to base 4 (3 characters for header) */
    for (i = 0; i < 3; i++) {
        int digit = (address >> (4 - i * 2)) & 3;
        switch (digit) {
            case 0: result[i] = 'a'; break;
            case 1: result[i] = 'b'; break;
            case 2: result[i] = 'c'; break;
            case 3: result[i] = 'd'; break;
        }
    }
    result[3] = '\0';
    return result;
}

/*
 * Converts a header code to a base 4 string.
 * Validates the input code and returns the corresponding base 4 string.
 */
char* header_code_to_base4(int code) {
    char *result = (char *)safe_malloc(3, "Memory allocation failed in header_code_to_base4");  /* 2 chars + null terminator */
    int i;
    if (!result) return NULL;

    /* Convert code to base 4 (2 characters for header) */
    for (i = 0; i < 2; i++) {
        int digit = (code >> (2 - i * 2)) & 3;
        switch (digit) {
            case 0: result[i] = 'a'; break;
            case 1: result[i] = 'b'; break;
            case 2: result[i] = 'c'; break;
            case 3: result[i] = 'd'; break;
        }
    }
    result[2] = '\0';
    return result;
}