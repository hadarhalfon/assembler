/*
 * word.c - Word data structure management
 * 
 * This file contains functions for managing Word structures, which represent
 * individual words of data or instructions in memory. It handles:
 * - Creating new word instances
 * - Adding words to linked lists
 * - Memory cleanup and destruction
 * - Address updates for data words
 */

#include "word.h"
#include "helpers.h"

#include <stdio.h>
#include <stdlib.h>

/*
 * new_word - Creates a new Word structure
 */
Word* new_word() {
    Word* tmp = (Word*)safe_malloc(sizeof(Word), "Memory allocation error");
    tmp->word = (char*)safe_malloc(11, "Memory allocation error for word field");
    tmp->word[0] = '\0'; /* Initialize to empty string */
    tmp->address = 0;
    tmp->type = 0;
    tmp->next = NULL;
    return tmp;
}

/*
 * add_word - Adds a new word to a linked list
 */
Word* add_word(Word** head, Word** tail) {
    Word* new_word = (Word*)safe_malloc(sizeof(Word), "Memory allocation failed in add_word");

    /* Allocate memory for the string (11 chars + '\0') */
    new_word->word = (char*)safe_malloc(11 * sizeof(char), "Memory allocation failed for word string");
    new_word->word[0] = '\0'; /* Initialize empty string */

    new_word->next = NULL;
    new_word->type = 0;
    new_word->address = 0; /* Initialize to 0 for consistency */

    if (*head == NULL) {
        *head = *tail = new_word;
    } else {
        (*tail)->next = new_word;
        *tail = new_word;
    }

    return new_word;
}

/*
 * destroy_word - Frees all memory allocated for a word list
 */
void destroy_word(Word* head) {
    if (head == NULL) {
        return;
    }
    while (head != NULL) {
        Word* tmp = head;
        head = head->next;
        free(tmp->word);
        free(tmp);
    }
}

/*
 * update_data - Updates addresses of data words
 */
void update_data(Word* word_head,int IC) {
    Word* curr_word = word_head;
    while (curr_word != NULL) {
        curr_word->address = curr_word->address + IC;
        curr_word = curr_word->next;
    }
}

