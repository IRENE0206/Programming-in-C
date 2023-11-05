#include "nuclei.h"

#ifndef EXTENSION
    void readFile(int argc, char *argv[], Nuclei *np);
#else
    void readInputs(int argc, char *argv[], Nuclei *np);
    void _countBrackets(char *s, int *open, int *close, bool *is, bool *id);
    void _validBrackets(int *open, int *close);
#endif
void separate(Nuclei *np, char *alltokens);
void _In(Nuclei *np, char c, char p, bool *in, int *t, int *i, char *q);
void _setBracket(Nuclei *np, int *token, int *i, char c);
void _setC(Nuclei *np, int token, int *i, char c);
void _setQuote(Nuclei *np, bool *in, int t, int *i, char c);
void _finishToken(Nuclei *np, int *token, int *i);
bool _isquote(char c);
bool _isbracket(char c);
void test(void);

int main(int argc, char *argv[])
{
    test();

    Nuclei *np = nuclei_init();

    #ifdef EXTENSION
        readInputs(argc, argv, np);
    #else
        readFile(argc, argv, np);
        Prog(np);
    #endif
    nuclei_free(&np);
    return 0;
}

#ifdef EXTENSION
    void readInputs(int argc, char *argv[], Nuclei *np)
    {
        if (argc != 1) {
            fprintf(stderr, "Usage: %s\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        char line[BIG];
        char alltokens[BIG];
        int open = 0, close = 0, i = 0;
        bool insingle = false, indouble = false;
        printf("Usage: type after the prompt\n");
        printf("To quit: Ctrl + C \n");
        while (open == 0 || open != close)
        {
            if (open == 0) {
                printf("nuclei> ");
            } else {
                assert(open != close);
                printf("      > ");
            }
            fgets(line, BIG, stdin);
            _countBrackets(line, &open, &close, &insingle, &indouble);
            _validBrackets(&open, &close);
            int length = strlen(line);
            if (line[length - 1] == '\n') {
                line[length - 1] = STREND;
                length -= 1;
            }
            strcpy(&(alltokens[i]), line);
            i += length;
            if (open == 0 && close == 0) {
                np->totalTokens = 0;
                np->currToken = 0;
                separate(np, alltokens);
                i = 0;
                alltokens[0] = STREND;
                assert(np->totalTokens < MAXTOKENNUM);
                Prog(np);
            }
        }
    }

    void _countBrackets(char *s, int *open, int *close, bool *is, bool *id)
    {
        int length = strlen(s);
        for (int j = 0; j < length; j++) {
            char c = s[j];
            if (!*is && !*id) {
                if (c == OPENBRACKET[0]) {
                    *open += 1;
                } else if (c == CLOSEBRACKET[0]) {
                    *close += 1;
                } else if (c == SINGLEQUOTE[0]) {
                    *is = true;
                } else if (c == DOUBLEQUOTE[0]) {
                    *id = true;
                }
            } else if (_isquote(c)) {
                assert(j > 0);
                char prev = s[j - 1];
                if (prev != '\\') {
                    if (c == SINGLEQUOTE[0]) {
                        *is = false;
                    } else {
                        assert(c == DOUBLEQUOTE[0]);
                        *id = false;
                    }
                }
            } 
        }
    }

    void _validBrackets(int *open, int *close)
    {
        if (*close > *open) {
            printf("Error: Unexpected )?");
        } else if (*open == *close) {
            *open = 0;
            *close = 0;
        }
    }
#endif

#ifndef EXTENSION
void readFile(int argc, char *argv[], Nuclei *np)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    FILE *fp = nfopen(argv[1], "r");
    char line[BIG];
    char alltokens[BIG];
    int i = 0;
    while (fgets(line, BIG, fp) != NULL) {
        int length = strlen(line);
        char last = line[length - 1];
        if (line[0] != '#') {
            if (length > 1 && line[length - 2] == '\r' && last == '\n') {
                line[length - 2] = STREND;
                length -= 2;
            } else if (length > 1 && last == '\n') {
                line[length - 1] = STREND;
                length -= 1;
            }
            strcpy(&(alltokens[i]), line);
            i += length;
        }
    }
    separate(np, alltokens);
    assert(np->totalTokens < MAXTOKENNUM);
    fclose(fp);
}
#endif

void separate(Nuclei *np, char *alltokens)
{
    bool indouble = false, insingle = false;
    int token = 0, i = 0, length = strlen(alltokens);
    for (int j = 0; j < length; j++) {
        char c = alltokens[j];
        if (!indouble && !insingle) {
            if (_isbracket(c)) {
                _setBracket(np, &token, &i, c);
            } else if (!_isquote(c) && !isspace(c)) {
                _setC(np, token, &i, c);
                i += 1;
                if (j < length - 1) {
                    char next = alltokens[j + 1];
                    if (_isquote(next) || _isbracket(next) || isspace(next)) {
                        _setC(np, token, &i, STREND);
                        _finishToken(np, &token, &i);
                    }
                }
            } else if (c == DOUBLEQUOTE[0]) {
                _setQuote(np, &indouble, token, &i, c);
            } else if (c == SINGLEQUOTE[0]) {
                _setQuote(np, &insingle, token, &i, c);
            } 
        } else {
            assert(j > 0);
            char prev = alltokens[j - 1];
            if (indouble) {
                _In(np, c, prev, &indouble, &token, &i, DOUBLEQUOTE);
            } else {
                assert(insingle);
                _In(np, c, prev, &insingle, &token, &i, SINGLEQUOTE);
            }
        }
    }
}

void _In(Nuclei *np, char c, char p, bool *in, int *t, int *i, char *q)
{
    if (p != '\\' && c == q[0]) {
        *in = false;
        _setC(np, *t, i, c);
        *i += 1;
        _setC(np, *t, i, STREND);
        _finishToken(np, t, i);
    } else {
        _setC(np, *t, i, c);
        *i += 1;
    }
}

void _setBracket(Nuclei *np, int *token, int *i, char c)
{
    _setC(np, *token, i, c);
    *i += 1;
    _setC(np, *token, i, STREND);
    _finishToken(np, token, i);
}

void _setC(Nuclei *np, int token, int *i, char c)
{
    (np->tokens)[token][*i] = c;
}

void _setQuote(Nuclei *np, bool *in, int t, int *i, char c)
{
    *in = true;
    _setC(np, t, i, c);
    *i += 1;
}

void _finishToken(Nuclei *np, int *token, int *i)
{
    np->totalTokens += 1;
    *token += 1;
    *i = 0;
}

bool _isquote(char c)
{
    return c == DOUBLEQUOTE[0] || c == SINGLEQUOTE[0];
}

bool _isbracket(char c)
{
    return c == OPENBRACKET[0] || c == CLOSEBRACKET[0];
}

void test()
{
    Nuclei *n1 = nuclei_init();
    separate(n1, "()");
    assert(n1->currToken == 0);
    assert(n1->totalTokens == 2);
    assert(_strcmp(getToken(n1), OPENBRACKET));
    next(n1);
    assert(n1->currToken == 1);
    assert(_strcmp(getToken(n1), CLOSEBRACKET));
    #ifdef INTERP
        assert(n1->lisp_count == 0);
        assert(n1->val_count == 0);
    #endif
    nuclei_free(&n1);
    assert(!n1);

    Nuclei *n2 = nuclei_init();
    separate(n2, "((SET A '2'))");
    assert(n2->currToken == 0);
    assert(n2->totalTokens == 7);
    assert(_strcmp(getToken(n2), OPENBRACKET));
    next(n2);
    assert(_strcmp(getToken(n2), OPENBRACKET));
    assert(n2->currToken == 1);
    next(n2);
    assert(n2->currToken == 2);
    char *s = (n2->tokens)[n2->currToken];
    assert(toSet(s));
    assert(toIOfunc(s));
    assert(toFunc(s));
    next(n2);
    assert(n2->currToken == 3);
    assert(isVar(getToken(n2)));
    assert(_strcmp(getToken(n2), "A"));
    Set(n2);
    #ifdef INTERP
        assert((n2->hasSet)[0]);
        assert(n2->val_count == 1);
        retVal *r2 = findVar(n2, "A");
        assert(r2);
        assert(_strcmp(r2->type, LISP));
        assert(lisp_isatomic(r2->lp));
        assert(n2->lisp_count == 1);
        lisp *l2 = (n2->lisp_generated)->latest;
        assert(l2);
        assert(lisp_getval(l2) == 2);
    #endif
    assert(_strcmp(getToken(n2), CLOSEBRACKET));
    next(n2);
    assert(n2->currToken == 6);
    assert(_strcmp(getToken(n2), CLOSEBRACKET));
    nuclei_free(&n2);
    assert(!n2);

    Nuclei *n3 = nuclei_init();
    separate(n3, "((PLUS '2' '3'))");
    assert(n3->currToken == 0);
    assert(n3->totalTokens == 7);
    next(n3);
    next(n3);
    char *s3 = getToken(n3);
    assert(_strcmp(s3, "PLUS"));
    assert(toIntfunc(s3));
    assert(toRetfunc(s3));
    assert(toFunc(s3));
    #ifdef INTERP
        next(n3);
        retVal *r3 = (retVal *) Intfunc(n3, s3);
        assert(r3);
        assert(n3->lisp_count == 3);
        assert(n3->val_count == 3);
        assert(lisp_getval(r3->lp) == 5);
    #else
        assert(Retfunc(n3, s3) == NULL);
        assert(_strcmp(getToken(n3), CLOSEBRACKET));
        next(n3);
        Instrcts(n3);
        assert(n3->currToken == n3->totalTokens);
    #endif
    nuclei_free(&n3);
    assert(!n3);

    Nuclei *n4 = nuclei_init();
    separate(n4, "((IF (EQUAL '1' '2') ((SET A NIL)) (SET B (CAR '(3 4)'))))");
    assert(n4->totalTokens == 25);
    next(n4);
    next(n4);
    assert(toIf(getToken(n4)));
    assert(toFunc(getToken(n4)));
    next(n4);
    next(n4);
    char *s4 = getToken(n4);
    assert(toBoolfunc(s4));
    next(n4);
    #ifdef INTERP
        retVal *r4 = (retVal *) Boolfunc(n4, s4);
        assert(r4);
        assert(_strcmp(r4->type, BOOL));
        assert(!(r4->b));
        assert(n4->lisp_count == 2);
        assert(n4->val_count == 3);
        next(n4);
        assert(_strcmp(getToken(n4), OPENBRACKET));
        next(n4);
        assert(_strcmp(getToken(n4), OPENBRACKET));
        skip(n4);
        assert(!(n4->hasSet)[0]);
        assert(n4->currToken == 15);
        assert(_strcmp(getToken(n4), OPENBRACKET));
        next(n4);
        s4 = getToken(n4);
        assert(toSet(s4));
        IOfunc(n4, s4);
        assert((n4->hasSet)['B' - 'A']);
        r4 = findVar(n4, "B");
        assert(lisp_getval(r4->lp) == 3);
    #else
        assert(Boolfunc(n4, s4) == NULL);
        next(n4);
        next(n4);
        next(n4);
        assert(toSet(getToken(n4)));
        next(n4);
        assert(isVar(getToken(n4)));
        next(n4);
        assert(isNil(getToken(n4)));
        next(n4);
        next(n4);
        next(n4);
        next(n4);
        assert(toSet(getToken(n4)));
        next(n4);
        assert(isVar(getToken(n4)));
        next(n4);
        next(n4);
        assert(toListfunc(getToken(n4)));
        next(n4);
        assert(isLiteral(getToken(n4)));
        next(n4);
    #endif
    assert(_strcmp(getToken(n4), CLOSEBRACKET));
    assert(_strcmp(getToken(n4), CLOSEBRACKET));
    assert(_strcmp(getToken(n4), CLOSEBRACKET));
    assert(_strcmp(getToken(n4), CLOSEBRACKET));
    nuclei_free(&n4);
    assert(!n4);

    Nuclei *n5 = nuclei_init();
    separate(n5, "((CONS '(1 2)' '(3 4)'))");
    assert(n5->totalTokens == 7);
    next(n5);
    next(n5);
    char *s5 = getToken(n5);
    assert(_strcmp(s5, "CONS"));
    assert(toListfunc(s5));
    assert(toRetfunc(s5));
    next(n5);
    s5 = getToken(n5);
    assert(_strcmp(s5, "'(1 2)'"));
    assert(isStrOrLiteral(s5, SINGLEQUOTE));
    assert(isLiteral(s5));
    assert(!isString(s5));
    #ifdef INTERP
        retVal *r5 = literalToLisp(n5, s5);
        assert(_strcmp(r5->type, LISP));
        assert(!lisp_isatomic(r5->lp));
        char str[BIG];
        lisp_tostring(r5->lp, str);
        assert(_strcmp(str, "(1 2)"));
    #endif
    next(n5);
    assert(n5->currToken == 4);
    s5 = getToken(n5);
    assert(_strcmp(s5, "'(3 4)'"));
    assert(isStrOrLiteral(s5, SINGLEQUOTE));
    assert(isLiteral(s5));
    assert(!isString(s5));
    #ifdef INTERP
        r5 = literalToLisp(n5, s5);
        assert(_strcmp(r5->type, LISP));
        assert(!lisp_isatomic(r5->lp));
        char str5[BIG];
        lisp_tostring(r5->lp, str5);
        assert(_strcmp(str5, "(3 4)"));
    #endif
    next(n5);
    assert(_strcmp(getToken(n5), ")"));
    next(n5);
    assert(_strcmp(getToken(n5), ")"));
    nuclei_free(&n5);
    assert(!n5);

    Nuclei *n6 = nuclei_init();
    char c6 = '(';
    assert(_isbracket(c6));
    int t6 = n6->currToken;
    int i6 = 0;
    assert(t6 == 0);
    _setBracket(n6, &t6, &i6, c6);
    assert(t6 == 1);
    assert(n6->totalTokens == 1);
    assert(i6 == 0);
    _setBracket(n6, &t6, &i6, c6);
    assert(t6 == 2);
    assert(n6->totalTokens == 2);
    assert(i6 == 0);
    strcpy(&((n6->tokens)[t6][i6]), "PRINT");
    n6->totalTokens += 1;
    t6 += 1;
    c6 = '"';
    assert(_isquote(c6));
    bool b6 = false;
    _setQuote(n6, &b6, t6, &i6, c6);
    assert(b6);
    assert(i6 == 1);
    _setC(n6, t6, &i6, 'X');
    assert(i6 == 1);
    i6 += 1;
    _In(n6, 'X', 'X', &b6, &t6, &i6, DOUBLEQUOTE);
    assert(i6 == 3);
    _In(n6, '"', 'X', &b6, &t6, &i6, DOUBLEQUOTE);
    assert(!b6);
    assert(t6 == 4);
    assert(i6 == 0);
    assert(_strcmp((n6->tokens)[t6 - 1], "\"XX\""));
    _setC(n6, t6, &i6, ')');
    i6 += 1;
    _setC(n6, t6, &i6, STREND);
    assert(_strcmp((n6->tokens)[t6], ")"));
    _finishToken(n6, &t6, &i6);
    assert(t6 == 5);
    _setBracket(n6, &t6, &i6, ')');
    assert(t6 == 6);
    assert(n6->totalTokens == t6);
    #ifndef INTERP
        assert(n6->currToken == 0);
    #endif
    nuclei_free(&n6);
    assert(!n6);
}
