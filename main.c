#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "tt.h"

int main(int argc, char **argv) {
    printf("STDC: %li\n", __STDC_VERSION__);

    if (argc < 2) {
        fprintf(stderr, "Incorrect number of arguments.");
        exit(1);
    }
    
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    printf("File: %s\n", argv[1]);

    struct lexer lexer = init_lexer(file);
    struct lexer_error *errors = malloc(sizeof(struct lexer_error) * 8);
    size_t num_errors = 8;

    struct token *tokens = lex(&lexer, errors, &num_errors);

    int num_tok = 0;
    printf("\nnumtok: %i numerr: %zu\n", num_tok, num_errors);
    printf("\n== Tokens ==\n");
    for (struct token *token = tokens; token->tt != TT_EOF; token++) {
        printf("token tt:%i l:%s @ %i:%i\n", token->tt, token->lexeme, token->line, token->col);
        num_tok++;
    }

    printf("next: %i", fgetc(file));

    printf("\n==Errors== \n");
    if (num_errors == 0) printf("No Errors\n");
    else
        for (size_t i = 0; i < num_errors; i++) {
            printf("error errno:%i @ %c @ %i:%i\n", errors[i].lerrno, errors[i].ch, errors[i].line, errors[i].col);
        }

    fclose(file);
    free(errors);
    free(tokens);
    return 0;
}
