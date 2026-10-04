#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#define MAX_WORDS 7
#define MAX_LEN 20
#define MAX_UNIQUE 10

typedef struct {
    char words[MAX_WORDS][MAX_LEN];
    int word_len[MAX_WORDS];
    int num_words;
    char result[MAX_LEN];
    int result_len;
} Puzzle;

int letter_map[26];
int used_digits[10];
char unique_letters[MAX_UNIQUE];
int num_unique;
int leading[26];

void parse(const char* input, Puzzle* p) {
    p->num_words = 0;
    memset(leading, 0, sizeof(leading));
    
    const char* eq = strchr(input, '=');
    if (!eq) return;
    
    char left[256], right[256];
    strncpy(left, input, eq - input);
    left[eq - input] = '\0';
    strcpy(right, eq + 1);
    
    int k = 0;
    for (int i = 0; right[i]; i++)
        if (!isspace((unsigned char)right[i])) right[k++] = right[i];
    right[k] = '\0';
    strcpy(p->result, right);
    p->result_len = strlen(right);
    if (p->result_len > 0) leading[right[0] - 'A'] = 1;
    
    char* lcopy = strdup(left);
    char* tok = strtok(lcopy, "+");
    while (tok && p->num_words < MAX_WORDS) {
        k = 0;
        for (int i = 0; tok[i]; i++)
            if (!isspace((unsigned char)tok[i])) p->words[p->num_words][k++] = tok[i];
        p->words[p->num_words][k] = '\0';
        p->word_len[p->num_words] = k;
        if (k > 0) leading[p->words[p->num_words][0] - 'A'] = 1;
        p->num_words++;
        tok = strtok(NULL, "+");
    }
    free(lcopy);
    
    int seen[26] = {0};
    num_unique = 0;
    for (int i = 0; i < p->num_words; i++)
        for (int j = 0; j < p->word_len[i]; j++) {
            char c = p->words[i][j];
            if (!seen[c - 'A']) {
                seen[c - 'A'] = 1;
                unique_letters[num_unique++] = c;
            }
        }
    for (int j = 0; j < p->result_len; j++) {
        char c = p->result[j];
        if (!seen[c - 'A']) {
            seen[c - 'A'] = 1;
            unique_letters[num_unique++] = c;
        }
    }
}

long long word_to_num(const char* word, int len) {
    long long num = 0;
    for (int i = 0; i < len; i++) {
        int d = letter_map[word[i] - 'A'];
        if (d == -1) return -1;
        num = num * 10 + d;
    }
    return num;
}

int check(const Puzzle* p) {
    long long sum = 0;
    for (int i = 0; i < p->num_words; i++) {
        long long v = word_to_num(p->words[i], p->word_len[i]);
        if (v == -1) return 0;
        sum += v;
    }
    long long res = word_to_num(p->result, p->result_len);
    return (res != -1 && sum == res);
}

int solve_naive(const Puzzle* p, int depth) {
    if (depth == num_unique) return check(p);
    
    char letter = unique_letters[depth];
    int idx = letter - 'A';
    
    for (int d = 0; d <= 9; d++) {
        if (d == 0 && leading[idx]) continue;
        if (used_digits[d]) continue;
        
        letter_map[idx] = d;
        used_digits[d] = 1;
        
        if (solve_naive(p, depth + 1)) return 1;
        
        letter_map[idx] = -1;
        used_digits[d] = 0;
    }
    return 0;
}

void print_result(const Puzzle* p) {
    for (int i = 0; i < p->num_words; i++) {
        for (int j = 0; j < p->word_len[i]; j++)
            printf("%d", letter_map[p->words[i][j] - 'A']);
        if (i < p->num_words - 1) printf(" + ");
    }
    printf(" = ");
    for (int j = 0; j < p->result_len; j++)
        printf("%d", letter_map[p->result[j] - 'A']);
    printf("\n");
}

int main() {
    const char* input = "SEND + MORE = MONEY";
    Puzzle p;
    
    memset(letter_map, -1, sizeof(letter_map));
    memset(used_digits, 0, sizeof(used_digits));
    
    clock_t start = clock();
    
    parse(input, &p);
    int found = solve_naive(&p, 0);
    
    clock_t end = clock();
    double time = (double)(end - start) / CLOCKS_PER_SEC;
    
    if (found) print_result(&p);
    else printf("No solution\n");
    
    printf("Time: %.6f sec\n", time);
    return 0;
}