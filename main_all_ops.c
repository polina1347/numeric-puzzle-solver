#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#define MAX_LEN 20
#define MAX_UNIQUE 10
#define MAX_TOKENS 20

typedef struct {
    int is_operator;
    char word[MAX_LEN];
    int word_len;
    char op;
} Token;

Token tokens[MAX_TOKENS];
int token_count;
char result_word[MAX_LEN];
int result_len;
int letter_map[26];
int used_mask;
char unique_letters[MAX_UNIQUE];
int num_unique;
int leading[26];
int letter_weight[26];
int parse_pos;

long long parse_expr(void);
long long parse_term(void);
long long parse_factor(void);

void parse_input(const char* input) {
    token_count = 0;
    num_unique = 0;
    memset(leading, 0, sizeof(leading));
    memset(letter_weight, 0, sizeof(letter_weight));
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
    strcpy(result_word, right);
    result_len = strlen(right);
    if (result_len > 0) leading[right[0] - 'A'] = 1;
    char* lcopy = strdup(left);
    char* pos = lcopy;
    int expect_word = 1;
    while (*pos) {
        while (*pos && isspace((unsigned char)*pos)) pos++;
        if (!*pos) break;
        if (expect_word) {
            k = 0;
            while (*pos && isalpha((unsigned char)*pos)) {
                tokens[token_count].word[k++] = *pos;
                pos++;
            }
            tokens[token_count].word[k] = '\0';
            tokens[token_count].word_len = k;
            tokens[token_count].is_operator = 0;
            if (k > 0) leading[tokens[token_count].word[0] - 'A'] = 1;
            token_count++;
            expect_word = 0;
        } else {
            if (*pos == '+' || *pos == '-' || *pos == '*' || *pos == '/') {
                tokens[token_count].op = *pos;
                tokens[token_count].is_operator = 1;
                tokens[token_count].word_len = 0;
                token_count++;
                pos++;
                expect_word = 1;
            } else {
                pos++;
            }
        }
    }
    free(lcopy);
    int seen[26] = {0};
    for (int i = 0; i < token_count; i++) {
        if (!tokens[i].is_operator) {
            for (int j = 0; j < tokens[i].word_len; j++) {
                char c = tokens[i].word[j];
                int idx = c - 'A';
                if (!seen[idx]) {
                    seen[idx] = 1;
                    unique_letters[num_unique++] = c;
                }
                int pos_in_word = tokens[i].word_len - 1 - j;
                int weight = 1;
                for (int m = 0; m < pos_in_word; m++) weight *= 10;
                letter_weight[idx] += weight;
            }
        }
    }
    for (int j = 0; j < result_len; j++) {
        char c = result_word[j];
        int idx = c - 'A';
        if (!seen[idx]) {
            seen[idx] = 1;
            unique_letters[num_unique++] = c;
        }
        int pos_in_word = result_len - 1 - j;
        int weight = 1;
        for (int m = 0; m < pos_in_word; m++) weight *= 10;
        letter_weight[idx] += weight;
    }
    for (int i = 0; i < num_unique - 1; i++) {
        for (int j = i + 1; j < num_unique; j++) {
            if (letter_weight[unique_letters[i] - 'A'] < letter_weight[unique_letters[j] - 'A']) {
                char temp = unique_letters[i];
                unique_letters[i] = unique_letters[j];
                unique_letters[j] = temp;
            }
        }
    }
}

long long word_value(const char* word, int len) {
    long long val = 0;
    for (int i = 0; i < len; i++) {
        int d = letter_map[word[i] - 'A'];
        if (d == -1) return -1;
        val = val * 10 + d;
    }
    return val;
}

long long parse_expr(void) {
    long long result = parse_term();
    while (parse_pos < token_count && tokens[parse_pos].is_operator &&
           (tokens[parse_pos].op == '+' || tokens[parse_pos].op == '-')) {
        char op = tokens[parse_pos].op;
        parse_pos++;
        long long right = parse_term();
        if (right < 0 || result < 0) return -1;
        if (op == '+') result += right;
        else result -= right;
    }
    return result;
}

long long parse_term(void) {
    long long result = parse_factor();
    while (parse_pos < token_count && tokens[parse_pos].is_operator &&
           (tokens[parse_pos].op == '*' || tokens[parse_pos].op == '/')) {
        char op = tokens[parse_pos].op;
        parse_pos++;
        long long right = parse_factor();
        if (right < 0 || result < 0) return -1;
        if (op == '*') result *= right;
        else {
            if (right == 0) return -1;
            if (result % right != 0) return -2;
            result /= right;
        }
    }
    return result;
}

long long parse_factor(void) {
    if (parse_pos >= token_count) return -1;
    if (tokens[parse_pos].is_operator) return -1;
    long long val = word_value(tokens[parse_pos].word, tokens[parse_pos].word_len);
    parse_pos++;
    return val;
}

long long evaluate_expression(void) {
    parse_pos = 0;
    return parse_expr();
}

int check(void) {
    long long left = evaluate_expression();
    if (left < 0) return 0;
    long long right = word_value(result_word, result_len);
    if (right < 0) return 0;
    return (left == right);
}

int solve(int depth) {
    if (depth == num_unique) return check();
    char letter = unique_letters[depth];
    int idx = letter - 'A';
    for (int d = 0; d <= 9; d++) {
        if (d == 0 && leading[idx]) continue;
        if (used_mask & (1 << d)) continue;
        letter_map[idx] = d;
        used_mask |= (1 << d);
        if (solve(depth + 1)) return 1;
        letter_map[idx] = -1;
        used_mask &= ~(1 << d);
    }
    return 0;
}

void print_result(void) {
    for (int i = 0; i < token_count; i++) {
        if (tokens[i].is_operator) {
            printf(" %c ", tokens[i].op);
        } else {
            for (int j = 0; j < tokens[i].word_len; j++)
                printf("%d", letter_map[tokens[i].word[j] - 'A']);
        }
    }
    printf(" = ");
    for (int j = 0; j < result_len; j++)
        printf("%d", letter_map[result_word[j] - 'A']);
    printf("\n");
}

int main(void) {
    const char* tests[][2] = {
        {"SEND + MORE = MONEY", "Addition"},
        {"AB - C = D", "Subtraction"},
        {"A * B = CD", "Multiplication"},
        {"AB / C = D", "Division"},
        {"AB + CD - E = FG", "Combined"}
    };
    int num_tests = 5;

    for (int t = 0; t < num_tests; t++) {
        printf("--- Test %d: %s (%s) ---\n", t + 1, tests[t][0], tests[t][1]);
        memset(letter_map, -1, sizeof(letter_map));
        used_mask = 0;
        clock_t start = clock();
        parse_input(tests[t][0]);
        int found = solve(0);
        clock_t end = clock();
        double time = (double)(end - start) / CLOCKS_PER_SEC;
        if (found) print_result();
        else printf("No solution\n");
        printf("Time: %.6f sec\n\n", time);
    }
    return 0;
}