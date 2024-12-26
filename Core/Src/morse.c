/*
 * morse.c
 *
 *  Created on: Dec 23, 2024
 *      Author: achille
 */
#include <morse.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define HANDLE_ALLOC_ERR(ptr)                                                                      \
    do {                                                                                           \
        if (!ptr) {                                                                                \
            return NULL;                                                                           \
        }                                                                                          \
    } while (0)

const enum morse_char_t MORSE_A[] = {Dot, Dash, End};
const enum morse_char_t MORSE_B[] = {Dash, Dot, Dot, Dot, End};
const enum morse_char_t MORSE_C[] = {Dash, Dot, Dash, Dot, End};
const enum morse_char_t MORSE_D[] = {Dash, Dot, Dot, End};
const enum morse_char_t MORSE_E[] = {Dot, End};
const enum morse_char_t MORSE_F[] = {Dot, Dot, Dash, Dot, End};
const enum morse_char_t MORSE_G[] = {Dash, Dash, Dot, End};
const enum morse_char_t MORSE_H[] = {Dot, Dot, Dot, Dot, End};
const enum morse_char_t MORSE_I[] = {Dot, Dot, End};
const enum morse_char_t MORSE_J[] = {Dot, Dash, Dash, Dash, End};
const enum morse_char_t MORSE_K[] = {Dash, Dot, Dash, End};
const enum morse_char_t MORSE_L[] = {Dot, Dash, Dot, Dot, End};
const enum morse_char_t MORSE_M[] = {Dash, Dash, End};
const enum morse_char_t MORSE_N[] = {Dash, Dot, End};
const enum morse_char_t MORSE_O[] = {Dash, Dash, Dash, End};
const enum morse_char_t MORSE_P[] = {Dot, Dash, Dash, Dot, End};
const enum morse_char_t MORSE_Q[] = {Dash, Dash, Dot, Dash, End};
const enum morse_char_t MORSE_R[] = {Dot, Dash, Dot, End};
const enum morse_char_t MORSE_S[] = {Dot, Dot, Dot, End};
const enum morse_char_t MORSE_T[] = {Dash, End};
const enum morse_char_t MORSE_U[] = {Dot, Dot, Dash, End};
const enum morse_char_t MORSE_V[] = {Dot, Dot, Dot, Dash, End};
const enum morse_char_t MORSE_W[] = {Dot, Dash, Dash, End};
const enum morse_char_t MORSE_X[] = {Dash, Dot, Dot, Dash, End};
const enum morse_char_t MORSE_Y[] = {Dash, Dot, Dash, Dash, End};
const enum morse_char_t MORSE_Z[] = {Dash, Dash, Dot, Dot, End};

const enum morse_char_t *MORSE_ALPHABET[26] = {
    MORSE_A, MORSE_B, MORSE_C, MORSE_D, MORSE_E, MORSE_F, MORSE_G, MORSE_H, MORSE_I,
    MORSE_J, MORSE_K, MORSE_L, MORSE_M, MORSE_N, MORSE_O, MORSE_P, MORSE_Q, MORSE_R,
    MORSE_S, MORSE_T, MORSE_U, MORSE_V, MORSE_W, MORSE_X, MORSE_Y, MORSE_Z};

MorseNode *create_node(char letter) {
    MorseNode *node = (MorseNode *)malloc(sizeof(MorseNode));
    HANDLE_ALLOC_ERR(node);
    node->letter = letter;
    node->dot = NULL;
    node->dash = NULL;
    return node;
}

MorseNode *insert_morse(MorseNode *root, const enum morse_char_t *sequence, char letter) {
    MorseNode *current = root;
    while (*sequence != End) {
        if (*sequence == Dot) {
            if (!current->dot) {
                current->dot = create_node('\0');
                HANDLE_ALLOC_ERR(current->dot);
            }
            current = current->dot;
        } else if (*sequence == Dash) {
            if (!current->dash) {
                current->dash = create_node('\0');
                HANDLE_ALLOC_ERR(current->dash);
            }
            current = current->dash;
        }
        sequence++;
    }
    current->letter = letter;
    return current;
}

MorseNode *initialize_morse_tree() {
    MorseNode *root = create_node('\0');
    HANDLE_ALLOC_ERR(root);
    for (uint8_t i = 0; i < sizeof(MORSE_ALPHABET) / sizeof(*MORSE_ALPHABET); i++) {
        HANDLE_ALLOC_ERR(insert_morse(root, MORSE_ALPHABET[i], 'A' + i));
    }
    return root;
}

char decode_morse(MorseNode *root, const enum morse_char_t *sequence) {
    MorseNode *current = root;
    while (*sequence != End && current) {
        if (*sequence == Dot) {
            current = current->dot;
        } else if (*sequence == Dash) {
            current = current->dash;
        }
        sequence++;
    }
    return current && current->letter ? current->letter : '\0';
}
