/*
 * word.h - Word data structure header
 * 
 * This header file defines the Word structure and related functions
 */

#ifndef WORD_H
#define WORD_H

/*
 * Word - Represents a word in memory
 * 
 * A word is the basic unit of data in the assembler's memory model.
 * Each word contains binary data, an address, a type, and a pointer
 * to the next word in a linked list.
 */
typedef struct Word {
    char* word;         /* Binary representation of the word (10 bits) */
    int address;        /* Memory address of this word */
    int type;           /* Type: 0=data, 1=instruction */
    struct Word* next;  /* Pointer to next word in list */
}Word;

/* Word management functions */
Word* new_word();                           /* Creates a new word */
Word* add_word(Word** head,Word** tail);   /* Adds word to list */
void destroy_word(Word* head);              /* Frees word list */
void update_data(Word* word_head,int IC);  /* Updates data addresses */

#endif /* WORD_H */