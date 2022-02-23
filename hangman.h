#ifndef __HANGMAN_H_
#define __HANGMAN_H_

#define MAX_NUM_WORDS				1000
#define MAX_WORD_LENGTH     100
#define MAX_NO_OF_ATTEMPTS   10
#define E_USER_LOST         110
#define E_USER_WON            0

#define DICTIONARY	"dictionary.dat"

/*
 *< Note: header has to be defined as a `char *' and 
 *< cols in context of the use of HEADER_START_COL 
 */
#define HEADER_START_ROW		3
#define HEADER_START_COL		(cols/2 - (strlen(header)/2) - 5)

char hm_dictionary[MAX_NUM_WORDS][MAX_WORD_LENGTH];

#endif //__HANGMAN_H_
