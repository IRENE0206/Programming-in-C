#pragma once

#include "general.h"

#include <assert.h>
#include <ctype.h>

#define BIG 100000
#define VALTYPE 10
#define MAXTOKENSIZE 100
#define MAXTOKENNUM 10000
#define LETTERS 'Z' - 'A' + 1
#define LISP "LISP"
#define BOOL "BOOL"
#define OPENBRACKET "("
#define CLOSEBRACKET ")"
#define SINGLEQUOTE "'"
#define DOUBLEQUOTE "\""
#define STREND '\0'

#ifdef INTERP
    typedef struct lisp lisp;

    typedef int atomtype;

    struct node {
        atomtype val;
        struct node *head;
        struct node *remainder;
    };
    typedef struct node node;

    typedef struct retVal
    {
        char type[VALTYPE];
        lisp *lp;
        bool b;
    } retVal;

    struct LList
    {
        void *latest;
        struct LList *previous;
    };
    typedef struct LList LList;
    
#endif

typedef struct Nuclei
{
    char tokens[MAXTOKENNUM][MAXTOKENSIZE];
    int currToken;
    int totalTokens;
    #ifdef INTERP
        bool hasSet[LETTERS];
        int lisp_count;
        LList *lisp_generated;
        int val_count;
        LList *val_generated;
        retVal *variables[LETTERS];
    #endif
} Nuclei;

void Prog(Nuclei *np);
void Instrcts(Nuclei *np);
void Instrct(Nuclei *np);
void Func(Nuclei *np, char *s);
void * Retfunc(Nuclei *np, char *s);
void * Listfunc(Nuclei *np, char *s);
void * Intfunc(Nuclei *np, char *s);
void * Boolfunc(Nuclei *np, char *s);
void IOfunc(Nuclei *np, char *s);
void Set(Nuclei *np);
void Print(Nuclei *np);
void If(Nuclei *np);
void Loop(Nuclei *np);
void * List(Nuclei *np);
bool isVar(char *s);
bool isNil(char *s);
bool isString(char *s);
bool isLiteral(char *s);
bool isStrOrLiteral(char *s, char *quote);
void * Literal(Nuclei *np, char *s);

bool toFunc(char *s);
bool toRetfunc(char *s);
bool toListfunc(char *s);
bool toIntfunc(char *s);
bool toBoolfunc(char *s);
bool toIOfunc(char *s);
bool toSet(char *s);
bool toPrint(char *s);
bool toIf(char *s);
bool toLoop(char *s);
bool _strcmp(char *s1, char *s2);
void next(Nuclei *np);
void checkBracket(Nuclei *np, char *bracket);
void checkError(bool expected, char *element);
char * getToken(Nuclei *np);
Nuclei * nuclei_init(void);
void nuclei_free(Nuclei **np);

#ifdef INTERP
    retVal * retVal_init(void);
    retVal * findVar(Nuclei *np, char *s);
    void trackRetval(Nuclei *np, retVal *v);
    void free_vals(Nuclei *np);
    void _freeVal(retVal **v);
    void trackLisp(Nuclei *np, lisp *l);
    void free_lisps(Nuclei *np);
    void checkType(retVal *v, char *t);
    void checkAtom(retVal *v);
    void setRetvalLisp(retVal *v, lisp *li);
    void setRetvalBool(retVal *v, bool boo);
    void skip(Nuclei *np);
    void printList(Nuclei *np);
    void * literalToLisp(Nuclei *np, char *s);

    lisp * lisp_atom(Nuclei *np, const atomtype a);
    lisp * lisp_cons(Nuclei *np, const lisp* l1,  const lisp* l2);
    lisp * lisp_car(const lisp* l);
    lisp * lisp_cdr(const lisp* l);
    atomtype lisp_getval(const lisp* l);
    bool lisp_isatomic(const lisp* l);
    int lisp_length(const lisp* l);
    void lisp_tostring(const lisp* l, char* str);
    lisp * lisp_fromstring(Nuclei *np, const char* str);
    void _free(void **v);
    lisp * node_init(void);
    void length_helper(const lisp *l, int *length);
    void tostring_helper(const lisp *l, char *str, bool brackets);
    lisp * fromstr_helper(Nuclei *np, const char *str, int length, int *index);
    void str_to_int(const char *str, int *start, int *end, atomtype *result);
    void check_valid(bool expected);
#endif
