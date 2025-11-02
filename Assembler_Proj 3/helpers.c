/*
 * helpers.c - Helper functions for the assembler
 * 
 * This file contains utility functions used throughout the assembler for:
 * - Memory management (safe allocation)
 * - String manipulation and parsing
 * - Symbol and directive validation
 * - Operand type checking
 * - File name manipulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "macros.h"
#include "helpers.h"
#include "decode.h"

/*
 * Allocates memory and exits with error message if allocation fails.
 * This prevents segmentation faults from NULL pointer dereferences.
 */
void* safe_malloc(size_t size, const char* error_msg) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        printf("ERROR: %s\n", error_msg);
        exit(1);
    }
    return ptr;
}

/*
 * Concatenates a base filename with an extension to create
 * a complete filename (e.g., "file" + ".ob" = "file.ob")
 */
char* create_filename(const char* base_name, const char* extension) {
    char* fullname = (char*)safe_malloc(strlen(base_name) + strlen(extension) + 1, "Out of memory");
    strcpy(fullname, base_name);
    strcat(fullname, extension);
    return fullname;
}

/*
 * Advances the index past spaces and tabs, returning the position
 * of the next non-whitespace character or special return values
 * for end-of-line, comma, or bracket characters.
 */
int space_skip(const char*line,int i) {
    while (line[i] == ' ' || line[i] == '\t') {
        i+=1;
    }
    if (line[i] == '\n' || line[i] == '\r' || line[i] == '\0')
        return -2;
    if (line[i] == ',')
        return -1;
    if (line[i] == '[')
        return -3;
    return i;
}

/*
 * Looks for a valid symbol followed by a colon (:) at the given position.
 * This identifies symbol definitions like "label:" in assembly code.
 */
int is_symbol_definition(const char *line, int i) {
    int symbol_end = is_symbol(line,i);
    if (symbol_end > 0 && line[symbol_end] == ':')
        return symbol_end;
    return 0;
}

/*
 * Checks if the string starting at position i follows symbol naming rules:
 * - Must start with a letter
 * - Can contain letters and digits
 * - Must end with valid delimiter (colon, newline, comma, bracket, space)
 */
int is_symbol(const char * line, int i) {
    if (!(isalpha(line[i]))){
        return 0;
    }
    while (isalnum(line[i])) {
        i+=1;
    }
    
    if (line[i] == ':' || line[i] == '\n' || line[i] == '\r' || line[i] == ',' || line[i] == '[' || line[i] ==' ' || line[i] == '\0')
        return i;

    return 0;
}



/*
 * Checks if the string starting at position i matches any of the
 * supported assembly directives (.data, .string, .mat, .extern, .entry)
 */
int is_directive(const char *line, int i){
    if (strncmp(line+i,".data ",5) ==0)
        return 1;
    if (strncmp(line+i,".string ",7) ==0)
        return 2;
    if (strncmp(line+i,".mat ",4) ==0)
        return 3;
    if (strncmp(line+i,".extern ",7) ==0)
        return 4;
    if (strncmp(line+i,".entry ",6) ==0)
        return 5;
    return 0;
}

/*
 * Checks if the string starting at position index represents a valid
 * integer number (with optional sign). Handles positive and negative numbers.
 */
int is_number(char* line, int index) {
    int i = index;
    if (line == NULL) {
        return 0;
    }

    if (line[i] == '+' || line[i] == '-') {
        i++;
    }

    if (!isdigit(line[i])) {
        printf("is_number: Not a digit after sign\n");
        return 0;
    }

    while (isdigit(line[i])) {
        i++;
    }

    /* After digits can be: space, comma, end of line, carriage return, or end of string */
    if (line[i] == '\0' || line[i] == '\n' || line[i] == '\r' || line[i] == ' ' || line[i] == ',') {
        return 1;
    }

    printf("is_number: Unexpected character after number: '%c'\n", line[i]);
    return 0;
}


/*
 * Checks if the data initialization starting at position i follows proper syntax:
 * - Numbers separated by commas
 * - No leading or trailing commas
 * - No double commas
 * - Valid number format for each value
 */
int is_legal_data_or_matrix_initialization(const char *line, int i, int line_num) {
    char tmp[MAX_NUM_LENGTH + 1]; /* Max 10-bit number as string */
    int j;

    while (line[i] != '\0' && line[i] != '\n') {
        j = 0;

        /* Skip leading whitespace */
        i = space_skip(line, i);

        /* Check for leading comma */
        if (line[i] == ',') {
            printf("Error: Leading comma at line %d\n", line_num);
            return 0;
        }

        /* Read number into tmp */
        while (!isspace(line[i]) && line[i] != ',' && line[i] != '\0' && line[i] != '\n' && j <= MAX_NUM_LENGTH) {
            tmp[j++] = line[i++];
        }

        if (j == 0) {
            printf("Error: Missing number or double comma at line %d\n", line_num);
            return 0;
        }

        tmp[j] = '\0';

        if (j > MAX_NUM_LENGTH) {
            printf("Error: Number too long at line %d\n", line_num);
            return 0;
        }

        if (!is_number(tmp, 0)) {
            printf("Error: Not a valid number: %s at line %d\n", tmp, line_num);
            return 0;
        }

        i = space_skip(line, i); /* skip space after number */

        if (line[i] == ',') {
            i++; /* skip the comma */
            i = space_skip(line, i); /* skip spaces after comma */
            if (line[i] == '\0' || line[i] == '\n') {
                printf("Error: Trailing comma at line %d\n", line_num);
                return 0;
            }
            /* Check for double comma after skipping spaces */
            if (line[i] == ',') {
                printf("Error: Double comma at line %d\n", line_num);
                return 0;
            }
        } else if (line[i] != '\0' && line[i] != '\n') {
            printf("Error: Missing comma between values at line %d\n", line_num);
            return 0;
        } else {
            /* End of line - this is valid */
            break;
        }
    }

    return 1;
}


/*
 * Checks if the string starting at position i follows proper string syntax:
 * - Must start with double quote
 * - Must end with double quote
 * - No characters after closing quote except whitespace
 */
int is_legal_string(const char *line, int i, int line_num) {
    if (line[i] != '\"') {
        printf("Error: String must start with '\"' at line %d\n", line_num);
        return 0;
    }

    i++;  /* Skip the opening quote */

    /* Look for closing quote */
    while (line[i] != '\0' && line[i] != '\"') {
        i++;
    }

    if (line[i] != '\"') {
        printf("Error: Missing closing '\"' in string at line %d\n", line_num);
        return 0;
    }

    i = space_skip(line, i + 1); /* Skip after closing quote */

    if (line[i] != '\0') {
        printf("Error: Unexpected characters after string at line %d\n", line_num);
        return 0;
    }

    return 1;
}


/*
 * Validates comma usage in data initialization strings:
 * - No comma at the beginning
 * - No comma at the end
 * - No double commas
 * - Missing commas between values
 */
int contains_invalid_commas(const char *line, int index) {
    int i = index;
    int last_char_comma = 0;
    int comma_found = 0;

    /* Skip leading whitespace */
    while (line[i] == ' ' || line[i] == '\t') i++;

    if (line[i] == ',') return 1;  /* comma at beginning */

    /* First pass: check for double commas and trailing comma */
    while (line[i] != '\n' && line[i] != '\0') {
        if (line[i] == ',') {
            if (last_char_comma) return 1;  /* double comma */
            last_char_comma = 1;
            comma_found = 1;
        } else if (line[i] != ' ' && line[i] != '\t') {
            last_char_comma = 0;
        }
        i++;
    }

    if (last_char_comma && comma_found) return 1;  /* comma at end */

    /* Second pass: check for missing commas between values */
    i = index;
    while (line[i] == ' ' || line[i] == '\t') i++;
    
    while (i < strlen(line) && line[i] != '\n' && line[i] != '\0') {
        /* Skip to next non-whitespace character */
        while (i < strlen(line) && (line[i] == ' ' || line[i] == '\t')) i++;
        if (i >= strlen(line) || line[i] == '\n' || line[i] == '\0') break;
        
        /* If we find a number */
        if (line[i] == '-' || line[i] == '+' || isdigit(line[i])) {
            /* Skip the number */
            if (line[i] == '-' || line[i] == '+') i++;
            while (i < strlen(line) && isdigit(line[i])) i++;
            
            /* Skip whitespace after number */
            while (i < strlen(line) && (line[i] == ' ' || line[i] == '\t')) i++;
            
            /* If we're not at the end and the next character is not a comma, check if it's another number */
            if (i < strlen(line) && line[i] != '\n' && line[i] != '\0' && line[i] != ',') {
                if (line[i] == '-' || line[i] == '+' || isdigit(line[i])) {
                    return 1;  /* missing comma between numbers */
                }
            }
        } else {
            i++;
        }
    }

    return 0;
}

/*
 * Validates comma usage in matrix initialization strings:
 * - No comma at the beginning
 * - No comma at the end
 * - No double commas
 */
int contains_invalid_matrix_commas(const char *line, int index) {
    int i = index;
    int last_char_comma = 0;
    int comma_found = 0;

    while (line[i] == ' ' || line[i] == '\t') i++;

    if (line[i] == ',') return 1;  /* comma at beginning */

    while (line[i] != '\n' && line[i] != '\0') {
        if (line[i] == ',') {
            if (last_char_comma) return 1;  /* double comma */
            last_char_comma = 1;
            comma_found = 1;
        } else if (line[i] != ' ' && line[i] != '\t') {
            last_char_comma = 0;
        }
        i++;
    }

    if (last_char_comma && comma_found) return 1;  /* comma at end */

    return 0;
}

/*
 * Analyzes the data directive to determine the specific type of comma error:
 * - Leading comma
 * - Trailing comma
 * - Double comma
 * - Missing comma between values
 */
int get_data_comma_error_type(const char *line, int index) {
    int i = index;
    int last_char_comma = 0;
    int comma_found = 0;

    while (line[i] == ' ' || line[i] == '\t') i++;

    if (line[i] == ',') return 1;  /* leading comma */

    while (line[i] != '\n' && line[i] != '\0') {
        if (line[i] == ',') {
            if (last_char_comma) return 3;  /* double comma */
            last_char_comma = 1;
            comma_found = 1;
        } else if (line[i] != ' ' && line[i] != '\t') {
            last_char_comma = 0;
        }
        i++;
    }

    if (last_char_comma && comma_found) return 2;  /* trailing comma */
    
    /* Check for missing comma between values */
    i = index;
    while (line[i] == ' ' || line[i] == '\t') i++;
    
    while (i < strlen(line) && line[i] != '\n' && line[i] != '\0') {
        /* Skip to next non-whitespace character */
        while (i < strlen(line) && (line[i] == ' ' || line[i] == '\t')) i++;
        if (i >= strlen(line) || line[i] == '\n' || line[i] == '\0') break;
        
        /* If we find a number */
        if (line[i] == '-' || line[i] == '+' || isdigit(line[i])) {
            /* Skip the number */
            if (line[i] == '-' || line[i] == '+') i++;
            while (i < strlen(line) && isdigit(line[i])) i++;
            
            /* Skip whitespace after number */
            while (i < strlen(line) && (line[i] == ' ' || line[i] == '\t')) i++;
            
            /* If we're not at the end and the next character is not a comma, check if it's another number */
            if (i < strlen(line) && line[i] != '\n' && line[i] != '\0' && line[i] != ',') {
                if (line[i] == '-' || line[i] == '+' || isdigit(line[i])) {
                    return 4;  /* missing comma between numbers */
                }
            }
        } else {
            i++;
        }
    }

    return 0;  /* no error */
}

/*
 * Checks if the matrix declaration starting at position i follows proper syntax:
 * - Must start with .mat directive
 * - Must have two dimension specifications in brackets
 * - Each dimension must be a valid number
 * - Must be followed by valid data initialization
 */
int is_legal_mat(const char * line, int i, int line_num) {
    char num[MAX_NUM_LENGTH+1];
    int j = 0;

    if (line[i] != '[') {
        printf("Error: Expected '[' after .mat at line %d\n", line_num);
        return 0;
    }

    i++;  /* Skip '[' */
    j = 0;
    while (line[i] != '\0' && line[i] != ']' && j < MAX_NUM_LENGTH) {
        num[j++] = line[i++];
    }
    num[j] = '\0';

    if (line[i] != ']') {
        printf("Error: Missing closing ']' in first dimension at line %d\n", line_num);
        return 0;
    }

    if (!is_number(num, 0)) {
        printf("Error: Invalid number in first matrix dimension at line %d\n", line_num);
        return 0;
    }
    if (str_to_int(num) <= 0) {
        printf("Error: Invalid number in first matrix dimension at line %d\n", line_num);
        return 0;
    }

    i++;  /* Skip ']' */

    if (line[i] != '[') {
        printf("Error: Expected second '[' after first dimension at line %d\n", line_num);
        return 0;
    }

    i++;  /* Skip '[' */
    j = 0;
    while (line[i] != '\0' && line[i] != ']' && j < MAX_NUM_LENGTH) {
        num[j++] = line[i++];
    }
    num[j] = '\0';

    if (line[i] != ']') {
        printf("Error: Missing closing ']' in second dimension at line %d\n", line_num);
        return 0;
    }

    if (!is_number(num, 0)) {
        printf("Error: Invalid number in second matrix dimension at line %d\n", line_num);
        return 0;
    }
    if (str_to_int(num) <= 0) {
        printf("Error: Invalid number in second matrix dimension at line %d\n", line_num);
        return 0;
    }

    i++;  /* Skip final ']' */
    i = space_skip(line, i);

    return is_legal_data_or_matrix_initialization(line, i, line_num);
}


/*
 * Checks if the string starting at position i represents a valid register:
 * - Must start with 'r' followed by a digit 0-7
 * - Must not be followed by alphanumeric characters
 * - Must end with valid delimiter (comma, newline, bracket, etc.)
 */
int is_register(const char *line, int i, int line_num) {
    int skip_result;
    
    if (line == NULL) {
        printf("is_register: line is NULL\n");
        return 0;
    }
    if (i < 0 || i >= strlen(line) - 1) {
        printf("is_register: index out of bounds\n");
        return 0;
    }
    
    if (line[i] == 'r' && line[i+1] >= '0' && line[i+1] <= '7') {
        i += 2;
        
        if (i >= strlen(line) || isalnum(line[i])) {
            return 0;
        }
        
        skip_result = space_skip(line, i);
        if (skip_result == -2)  /* newline or end of string */
            return 2;
        if (skip_result == -1)  /* comma */
            return 1;
        if (line[i] == ']')
            return 3;
        if (line[i] == '\n' || line[i] == '\0')
            return 2;  /* Accept newline as valid delimiter */

        /* Check if there's a missing comma after register */
        if (line[i] != ' ' && line[i] != '\t' && line[i] != ',' && line[i] != '\n' && line[i] != '\0') {
            printf("Error: Missing comma after register at line %d\n", line_num);
        } else {
            printf("Error: Invalid register format at line %d\n", line_num);
        }
    }
    return 0;
}

/*
 * Checks if the string starting at position i represents a valid matrix operand:
 * - Must start with a valid symbol name
 * - Must be followed by two bracket-enclosed register indices
 * - Each index must be a valid register (r0-r7)
 * - Must end with valid delimiter (comma, newline, etc.)
 */
int is_mat_operand(const char *line, int i, int line_num) {
    int j;
    i = is_symbol(line,i);
    if (i == 0) {
        return 0;
    }
    for (j = 0; j < 2 ; j++) {
        if (line[i] != '[') {
            if ( j == 1)
                printf("Error: Invalid matrix operand format at line %d\n", line_num);
            return 0;
        }
        i += 1;
        if ( is_register(line,i, line_num) != 3) {
            printf("Error: Invalid matrix index at line %d\n", line_num);
            return 0;
        }

        i += 3;
    }
    if (space_skip(line,i) == -1)
        return 1;
    if (space_skip(line,i) == -2)
        return 2;
    if (space_skip(line,i) != i) {
        /* Check if there's a missing comma after matrix operand */
        if (line[i] != ' ' && line[i] != '\t' && line[i] != ',' && line[i] != '\n' && line[i] != '\0') {
            printf("Error: Missing comma after matrix operand at line %d\n", line_num);
        } else {
            printf("Error: Invalid matrix operand syntax at line %d\n", line_num);
        }
    }
    return 0;
}

/*
 * Checks if the string starting at position i represents a valid immediate operand:
 * - Must start with '#' character
 * - Must be followed by a valid number
 * - Must end with valid delimiter (comma, newline, space, etc.)
 */
int is_direct(const char *line, int i, int line_num) {
    char num[MAX_NUM_LENGTH+1];
    int j=0;
    
    if (line[i] != '#')
        return 0;
    i += 1;
    while (line[i] != '\n' && line[i] != '\r' && line[i] != ',' && line[i] != ' '  && j <= MAX_NUM_LENGTH && line[i] != '\0') {
        num[j] = line[i];
        i+=1;
        j+=1;
    }
    num[j] = '\0';
    
    if (!is_number(num, 0)) {
        printf("Error: Invalid immediate value at line %d\n", line_num);
        return 0;
    }
    if (space_skip(line,i) == -1)
        return 1;
    if (space_skip(line,i) == -2)
        return 2;
    /* Check if there's a missing comma after immediate value */
    if (line[i] != ' ' && line[i] != '\t' && line[i] != ',' && line[i] != '\n' && line[i] != '\0') {
        printf("Error: Missing comma after immediate value at line %d\n", line_num);
    } else {
        printf("Error: Invalid immediate addressing syntax at line %d\n", line_num);
    }
    return 0;

}

/*
 * Parses a matrix declaration line to extract the two dimensions
 * and calculates the total memory space needed (rows * columns).
 * Validates that both dimensions are non-negative.
 */
int save_place(char *line, int line_num) {
    int index = 0;
    int num1, num2;
    while (line[index] != '[') {
        index ++;
    }
    index += 1;
    index = space_skip(line,index);
    num1 = str_to_int(line + index);
    while (line[index] != '[') {
        index ++;
    }
    index += 1;
    index = space_skip(line,index);
    num2 = str_to_int(line + index);
    if ( num1 < 0 || num2 < 0) {
        printf("Error: Invalid matrix dimensions at line %d\n", line_num);
        return -1;
    }
    return num1 * num2;
}