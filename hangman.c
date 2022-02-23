/**
 * @Author Manoj Awasthi
 * June' 2006
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <sys/time.h>

#include "hangman.h"

/* you can also use different categories of words : 
 *				o EASY
 *				o MEDIUM
 *				o HARD
 */

/* different rating will also do wonder. have some rating for each user
 * according to the difficulty of word, no of attempts and `time' that user 
 * takes to play the game ... also something for timeout. 
 */

/* 
 * option shud be to complete the word in one guess as well .... 
 */


/* some global definitions */
int lines, cols;
int count_sigint = 0;

static char 	*header = "HANGMAN - Word-'o'-Mania";
static char 	*sajwat = "++++++++++++++++++++++++";

static char 	*program_name = NULL;
int 					dictionary_length = 0; 

static char getinput()  { char ch; 	noecho(); ch = getch();	 return ch; }

void display(char *astr, char invch) 
{
	int i, x, y;

	move(HEADER_START_ROW + 5, HEADER_START_COL);
	addstr("word: ");

	for(i=0;i<strlen(astr);i++)
		printw("%c%c", ((astr[i] == invch) ? '_': astr[i]), ' ');

	printw("\n");
}


static void finish (int sig) 
{
	endwin();

	/* Dont get confused this is planned to work only in case we become multi threaded some day */
	if (sig != 0 ) { 
		if(count_sigint) {
			printf("Fool, I am already handling a Ctrl C... dont u know that!!\n");
			return;
		}

		count_sigint++;	

		printf("Ctrl C received. handling ...\n");
	}

	exit(0);
}

int prep_dictionary () 
{
	int 	rc = 0;
	int		i = 0;
	char 	word[MAX_WORD_LENGTH];
	char	*rword = word;

  FILE  *fp;

	memset(word, '\0', MAX_WORD_LENGTH);
                                                                                                       
  fp = fopen(DICTIONARY, "r");
  if (fp == NULL) {
		clear();
    mvprintw(HEADER_START_ROW + 20, HEADER_START_COL - 20, "%s: could not open the dictionary file. won't continue."
          " Press any key to exit.\n", program_name);
    getch();
    rc = -1;
		goto out;
  }
  
  while(!feof(fp)) {
      while(fread(rword, 1, 1, fp) == 1) {
				rword++;
				if (*(rword-1) == '\n') {
					*(rword - 1) = '\0';
					strcpy(hm_dictionary[i++], word);
					memset(word, '\0', MAX_WORD_LENGTH);
					rword = word;
				}
      }
  }

	dictionary_length = i; /* not i + 1 */

	fclose(fp);

out:                                                                                                                
	return rc;
}

/* 
 * this function gets a random word from the array that we have 
 * prepared from the dictionary.dat file. 
 */
char * get_random_word () 
{
	struct timeval		tv;
	struct timezone 	tz;

	/* 
   * following code simply generates a random rindex 
	 * between 0 and dictionary length - 1. see man rand. 
 	 * Note: typecasting to double is required so that C takes 
	 *       care of producing a double result. otherwise probably
	 * 			 you end up getting 0 every time.
	 */
	gettimeofday(&tv, &tz); 
	srand(tv.tv_usec);

	return hm_dictionary[(int) ((double)dictionary_length*rand()/(RAND_MAX + 1.0))];
}

#if 0
int main () 
{
	int i;
	prep_dictionary();

	for(i=0;i<dictionary_length;i++)
		printf("B: %s\n", hm_dictionary[i]);
		

	for (i=0;i<80;i++) 
		printf("A: %s\n", get_random_word());

}
#endif

int main (int argc, char *argv[]) 
{
	int 	 rc = 0;
	char   ch;
	char   *qword = NULL;
	int 	 myargc = argc;
	char **myargv = argv;

	program_name = argv[0];


	(void) signal(SIGINT, finish);	// SIGINT handling code
		
	(void) initscr();								// init the curses lib
	(void) nonl();									// do not do nl => cr/nl on o/p
	(void) cbreak();								// take input char one at a time 
																	// do not wait for \n

	if (has_colors())
	{
		start_color();
		/*
		 * Simple color assignment, often all we need.
		 */
		init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
		init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
		init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
		init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
		init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
		init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
		init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
	}	

	/* on my system this is 41 and 125 */
	lines = LINES;
	cols  = COLS;

	rc = prep_dictionary();
	if (rc == -1) { 
		goto out_finish;
	}

	clear();

	while (1) {
		qword = get_random_word();
		if (qword == NULL) break;

  	rc = hangman_main(myargc, myargv, qword);
		clear();
		
		rc = attron(A_BLINK | A_BOLD);
		mvprintw(HEADER_START_ROW + 20, HEADER_START_COL - 10, "Do you wish to continue another game ? (y/n) ");
		ch = getinput();
		if (ch == 'n' || ch == 'N') {
			mvprintw(HEADER_START_ROW + 21, HEADER_START_COL - 10, "Ok. Bye, Have a Nice day. Press any key to exit.\n");
			ch = getch();
			break;
		}
		rc = attroff(A_BLINK | A_BOLD);
	}

out_finish:
	finish(0);
	return rc;
}

/*
 * this function simply displays a blank word and processes the game 
 * as per the rules. So this shud take the word as an arg in future.
 * 
 * future has come.
 */

int hangman_main(int argc, char *argv[], char *qword)
{
	int rc = 0;
	char invch = '?';
	char cnt, ch;
	short attempts = 0;
	int y, x;
	int i, j, k, len, FOUND;
	//char *qword = "ostentatious";
	char aword[MAX_WORD_LENGTH];

	/* display the header */
	move(HEADER_START_ROW, HEADER_START_COL);
	addstr(header);

	move(HEADER_START_ROW + 1, HEADER_START_COL);
	addstr(sajwat);

	len = strlen(qword);
	/* 
	 * find an invalid character from the alphabets 
	 * if possible. anf if not found then use `?' 
   */
	FOUND = 0;
	for(cnt='a';cnt<='z';cnt++) {
		for(i=0;i<len;i++) {
			if (cnt == qword[i])
				FOUND = 1;
		}
		if (FOUND == 0) {
			invch = cnt;
			break;
		}
	}

	memset(aword, '\0', MAX_WORD_LENGTH);
	memset(aword, invch, len);

	display(aword, invch);

	while(strcmp(qword, aword)) { 

		ch = getinput();

#if 0
		if(ch == '?') {
			getyx(stdscr, y, x);		// this is a macro, so no '&'
			mvprintw(HEADER_START_ROW + 20, HEADER_START_COL, "%s: invalid input. you entered '%c'\n", 
					program_name, ch);
			move(y,x);
			continue;
		}
#endif

		if (attempts <= MAX_NO_OF_ATTEMPTS) { 
			FOUND = 0;
			for(i=0;i<len;i++) { 
				if(qword[i] == ch) {
					aword[i] = ch;
					FOUND = 1;
				}
			} /* for(i=0 ... */
			if (FOUND == 0)
				attempts ++;
		} else {
			/* we are inside loop - means - we still did not get the 
			 * expected answer. attempts are > MAX ATTEMPTS. 
			 * So say `Sorry' (a way of showing courtesy to loser). Bail out.
			 */
			getyx(stdscr, y, x);							// this is a macro, so no '&'
			rc = attron(A_BLINK | A_BOLD);
			mvprintw(HEADER_START_ROW + 20, HEADER_START_COL - 10, "Sorry, you lost this game. you have exceeded the no."
					" of wrong attempts\n");
			mvprintw(HEADER_START_ROW + 21, HEADER_START_COL - 10, "Better luck next time. Good Bye."
					" Have a Nice day !\n");		
			mvprintw(HEADER_START_ROW + 22, HEADER_START_COL - 10, "Word: %s", qword);		
			mvprintw(HEADER_START_ROW + 23, HEADER_START_COL - 10, "Press any key to continue ...\n");
			rc = attroff(A_BLINK | A_BOLD);

			getch();

			move(y, x);

			rc = E_USER_LOST;
			goto out;
		}

		display(aword, invch);

	} /* while(strcmp ... */

	rc = E_USER_WON;
	getyx(stdscr, y, x);		// this is a macro, so no '&'
	rc = attrset(A_BLINK | A_BOLD);
	mvprintw(HEADER_START_ROW + 20, HEADER_START_COL - 10, "Congratulations !. You are really good !\n");
	mvprintw(HEADER_START_ROW + 21, HEADER_START_COL - 10, "No. of wrong attempts that you used was: %d\n", attempts);
	mvprintw(HEADER_START_ROW + 22, HEADER_START_COL - 10, "See you ! Good Bye. Have a Nice day !\n");

	mvprintw(HEADER_START_ROW + 22, HEADER_START_COL - 10, "Press any key to continue ...\n");
	rc = attroff(A_BLINK | A_BOLD);

	getch();
out:
	return rc;
}
