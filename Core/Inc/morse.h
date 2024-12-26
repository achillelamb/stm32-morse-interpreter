/*
 * morse.h
 *
 *  Created on: Dec 23, 2024
 *      Author: achille
 */

#ifndef INC_MORSE_H_
#define INC_MORSE_H_

enum morse_char_t {
    Dot,
    Dash,
    End,
};

typedef struct morse_node_t {
    char letter;
    struct morse_node_t *dot;
    struct morse_node_t *dash;
} MorseNode;

#define MORSE_CHAR_SIZE (sizeof(enum morse_char_t) * 8)
#define nullate(mchar) (memset(mchar, End, MORSE_CHAR_SIZE))
#define morse_eq(m1, m2) (memcmp(m1, m2, MORSE_CHAR_SIZE) == 0)

extern const enum morse_char_t MORSE_A[];
extern const enum morse_char_t MORSE_B[];
extern const enum morse_char_t MORSE_C[];
extern const enum morse_char_t MORSE_D[];
extern const enum morse_char_t MORSE_E[];
extern const enum morse_char_t MORSE_F[];
extern const enum morse_char_t MORSE_G[];
extern const enum morse_char_t MORSE_H[];
extern const enum morse_char_t MORSE_I[];
extern const enum morse_char_t MORSE_J[];
extern const enum morse_char_t MORSE_K[];
extern const enum morse_char_t MORSE_L[];
extern const enum morse_char_t MORSE_M[];
extern const enum morse_char_t MORSE_N[];
extern const enum morse_char_t MORSE_O[];
extern const enum morse_char_t MORSE_P[];
extern const enum morse_char_t MORSE_Q[];
extern const enum morse_char_t MORSE_R[];
extern const enum morse_char_t MORSE_S[];
extern const enum morse_char_t MORSE_T[];
extern const enum morse_char_t MORSE_U[];
extern const enum morse_char_t MORSE_V[];
extern const enum morse_char_t MORSE_W[];
extern const enum morse_char_t MORSE_X[];
extern const enum morse_char_t MORSE_Y[];
extern const enum morse_char_t MORSE_Z[];

MorseNode* initialize_morse_tree(void);
char decode_morse(MorseNode* root, const enum morse_char_t* sequence);

#endif /* INC_MORSE_H_ */
