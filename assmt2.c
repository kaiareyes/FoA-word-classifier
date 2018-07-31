/* Implements a named-entity recognition algorithm based on a entity name 
 * dictionary. Only labels person names.
 *
 * Written by Karina Andrea Reyes
 * Student Number 918552
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define WORDLENGTH 31
#define PROBLENGTH 3
#define INITIAL 200
#define HIGHPROB 80

#define ENDOFDIC "%%%%%%%%%%\n"
#define LINE1 "=========================Stage 1========================="
#define LINE2 "=========================Stage 2========================="
#define LINE3 "=========================Stage 3========================="
#define LINE4 "=========================Stage 4========================="
#define LINE5 "=========================Stage 5========================="
#define SPACE " "

#define NN 0
#define FN 1
#define LN 2
#define FNLN 3

#define FN_POS 0
#define LN_POS 1
#define NN_POS 2

#define NN_MESSAGE "NOT_NAME"
#define FNLN_MESSAGE "FIRST_NAME, LAST_NAME"
#define LN_MESSAGE "LAST_NAME"
#define FN_MESSAGE "FIRST_NAME"

/****************************************************************/

typedef char data_t[WORDLENGTH];
typedef struct node node_t;

struct node {
	data_t data;
	node_t *next;
};

typedef struct {
	node_t *head;
	node_t *foot;
} list_t;

typedef struct {
	data_t word;
	int prob[PROBLENGTH];
} one_word_t;

/****************************************************************/

/* function prototypes */

int read_word(one_word_t *a_word);
void print_stage1(one_word_t dict);
void print_stage2(int num_words, int total_ch);
void print_stage3(node_t * head);
int getword(char W[], int limit);
list_t *make_empty_list(void);
int is_empty_list(list_t *list);
void free_list(list_t *list);
list_t *insert_at_foot(list_t *list, data_t value);
int bs_stage4(one_word_t A[], int lo, int hi, data_t key, int *locn);
int bs_stage5(one_word_t A[], int lo, int hi, data_t key, int *locn);
void print_label(data_t a_word, int class);
void imp_bs(node_t *head, one_word_t *dict, int num_words, char *header,
			int (*bs) (one_word_t*, int, int, data_t, int*));

/****************************************************************/

int
main(int argc, char *argv[]) {
	one_word_t *dictionary;
	list_t *sentence;
	data_t sen_word;
	int i=0, num_words=0, total_ch=0;
	
	/* allocates memory to store a dictionary */
	dictionary = (one_word_t*)malloc(INITIAL*sizeof(*dictionary));
	assert(dictionary);
	
	/* loop through dict and read each word */
	for (i = 0; i<INITIAL; i++) { 
		read_word(dictionary+i);
		if (strcmp(dictionary[i].word, "")) { /* if not empty */
			num_words++;
			total_ch += strlen(dictionary[i].word);
		}
	}
	
	/* read a sentence, break into words, and store in a linked list */
	sentence = make_empty_list();
	while (getword(sen_word, WORDLENGTH) != EOF) {
		sentence = insert_at_foot(sentence, sen_word);
	}
	
	/* printing out Stage 1-3 */
	print_stage1(dictionary[0]);
	print_stage2(num_words, total_ch);
	print_stage3(sentence->head);
	
	/* implemements a binary search algorithm to label names, passing a
	function following classification rules for either stage 4 or 5 */
	imp_bs(sentence->head, dictionary, num_words, LINE4, bs_stage4);
	imp_bs(sentence->head, dictionary, num_words, LINE5, bs_stage5);

	/* free allocated memory to avoid memory leak */
	free_list(sentence);
	sentence = NULL;
	free(dictionary);
	dictionary = NULL;
	
	return 0;
}

/****************************************************************/

/* reads one entry in a name dictionary */
int
read_word(one_word_t *a_word) {
	while (strcmp(a_word->word, ENDOFDIC)) { 
		scanf(" #%[^\n]", a_word->word);
		scanf("%d %d %d",
		&a_word->prob[FN_POS],
		&a_word->prob[LN_POS],
		&a_word->prob[NN_POS]);
		return 0;
	}
	return 0;
}

/* prints the answer to Stage 1 */
void
print_stage1(one_word_t a_word) {
	printf("%s\n", LINE1);
	printf("Name 0: %s\n", a_word.word);
	printf("Label probabilities: %d%% %d%% %d%%\n", a_word.prob[FN_POS], 
		a_word.prob[LN_POS], a_word.prob[NN_POS]);
}

/* prints the answer to Stage 2 */
void
print_stage2(int num_words, int total_ch) {
	printf("\n%s\n", LINE2);
	printf("Number of names: %d\n", num_words);
	printf("Average number of characters per name: %.2f\n", 
		(float)total_ch/num_words);
}

/* iterates over a linked list and prints each word */
void 
print_stage3(node_t *head) {
	node_t *current = head;
	printf("\n%s\n", LINE3);
	while (current != NULL) {
		printf("%s\n", current->data);
		current = current->next;
	}
}

/* implements a binary search algorithm for Stage 4 and 5 given a function
   pointer */
void
imp_bs(node_t *head, one_word_t *dict, int num_words, char *header,
		int (*bs) (one_word_t*, int, int, data_t, int*)) {
	int locn, class;
	data_t a_word;
	node_t *current = head;
	printf("\n%s\n", header);
	
	/* look up each word from the sentence in the dictionary */
	while (current != NULL) { 
		strcpy(a_word, current->data);
		class = bs(dict, 0, num_words, a_word, &locn);/* int classification */
		print_label(a_word, class);
		current = current->next;
	}
}

/* prints the corresponding label given an integer classification */
void
print_label(data_t a_word, int class) {
	int i;
	printf("%s", a_word);
	for (i = 0; i<=WORDLENGTH-strlen(a_word); i++) { /* align label column */
		printf(" ");
	}
	if (class == NN) {
		printf("%s\n", NN_MESSAGE);
	} else if (class == FNLN) {
		printf("%s\n", FNLN_MESSAGE);
	} else if (class == LN) {
		printf("%s\n", LN_MESSAGE);
	} else {
		printf("%s\n", FN_MESSAGE);
	}
}

/****************************************************************/

/* Extract a single word out of the standard input, of not
   more than limit characters. Function taken from Figure 7.13 in the
   textbook. Written by Alistair Moffat. */
int
getword(char W[], int limit) {
	int c, len=0;
	/* first, skip over any non alphabetics */
	while ((c=getchar())!=EOF && !isalpha(c)) {
		/* do nothing more */
	}
	if (c==EOF) {
		return EOF;
	}
	/* ok, first character of next word has been found */
	W[len++] = c;
	while (len<limit && (c=getchar())!=EOF && isalpha(c)) {
		/* another character to be stored */
		W[len++] = c;
	}
	/* now close off the string */
	W[len] = '\0';
	return 0;
}

/****************************************************************/

/* Functions taken from listops.c in the textbook, originally written by 
   Alistair Moffat. I slightly modified it to fit strings. */

/* create an empty linked list */
list_t
*make_empty_list(void) {
	list_t *list;
	list = (list_t*)malloc(sizeof(*list));
	assert(list!=NULL);
	list->head = list->foot = NULL;
	return list;
}

/* check if list is empty */
int
is_empty_list(list_t *list) {
	assert(list!=NULL);
	return list->head==NULL;
}

/* free memory allocated to list */
void
free_list(list_t *list) {
	node_t *curr, *prev;
	assert(list!=NULL);
	curr = list->head;
	while (curr) {
		prev = curr;
		curr = curr->next;
		free(prev);
	}
	free(list);
}

/* insert new item at the end of the list */
list_t
*insert_at_foot(list_t *list, data_t value) {
	node_t *new;
	new = (node_t*)malloc(sizeof(*new));
	assert(list!=NULL && new!=NULL);
	strcpy(new->data, value);
	new->next = NULL;
	if (list->foot==NULL) {
		/* this is the first insertion into the list */
		list->head = list->foot = new;
	} else {
		list->foot->next = new;
		list->foot = new;
	}
	return list;
}

/****************************************************************/

/* Adopted from the binary_search() function in page 206 of the textbook. */

/* Searches for a word from the dictionary. Checks first two probabilities and
   classifies it as a first name, last name, both, or not a name. */
int
bs_stage4(one_word_t A[], int lo, int hi, data_t key, int *locn) {
	int mid, outcome;
	/*if key is in A, it is between A[lo] and A[hi-1] */
	if (lo>=hi) { /* not found */
		return NN;
	}
	mid = (lo+hi)/2;
	if ((outcome = strcmp(key, (A+mid)->word)) < 0) {
		return bs_stage4(A, lo, mid, key, locn);
	} else if (outcome > 0) {
		return bs_stage4(A, mid+1, hi, key, locn);
	} else { /* found */
		*locn = mid;
		if (((A+mid)->prob[FN_POS] > 0) && ((A+mid)->prob[LN_POS] > 0)) {
			return FNLN;
		} else if ((A+mid)->prob[LN_POS] > 0) {
			return LN;
		} else {
			return FN;
		}
	}
}

/* More refined labelling. Checks all probabilities and determines a single 
   label. */
int
bs_stage5(one_word_t A[], int lo, int hi, data_t key, int *locn) {
	int mid, outcome;
	/*if key is in A, it is between A[lo] and A[hi-1] */
	if (lo>=hi) { /* not found */
		return NN;
	}
	mid = (lo+hi)/2;
	if ((outcome = strcmp(key, (A+mid)->word)) < 0) {
		return bs_stage5(A, lo, mid, key, locn);
	} else if (outcome > 0) {
		return bs_stage5(A, mid+1, hi, key, locn);
	} else { /* found */
		*locn = mid;
		if (((A+mid)->prob[NN_POS] > (A+mid)->prob[LN_POS]) && 
			((A+mid)->prob[NN_POS] > (A+mid)->prob[FN_POS])) {
			return NN;
		} else if ((A+mid)->prob[LN_POS] > HIGHPROB) {
			return LN;
		} else {
			return FN;
		}
	}
}

/* algorithms are fun! */
