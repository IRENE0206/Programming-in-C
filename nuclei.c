#include "nuclei.h"

void Prog(Nuclei *np)
{
    checkBracket(np, OPENBRACKET);
    Instrcts(np);
    #ifndef INTERP
        printf("Parsed OK\n");
    #endif
}

void Instrcts(Nuclei *np)
{
    char *s = getToken(np);
    if (_strcmp(s, CLOSEBRACKET)) {
        next(np);
        return;
    } else {
        Instrct(np);
        Instrcts(np);
    }
}

void Instrct(Nuclei *np)
{
    checkBracket(np, OPENBRACKET);
    char *s = getToken(np);
    checkError(toFunc(s), "FUNC");
    Func(np, s);
    checkBracket(np, CLOSEBRACKET);
}

void Func(Nuclei *np, char *s)
{
    if(toRetfunc(s)) {
        Retfunc(np, s);
    } else if (toIOfunc(s)) {
        IOfunc(np, s);
    } else if (toIf(s)) {
        If(np);
    } else {
        assert(toLoop(s));
        Loop(np);
    }
}

void * Retfunc(Nuclei *np, char *s)
{
    if (toListfunc(s)) {
        next(np);
        return Listfunc(np, s);
    } else if (toIntfunc(s)) {
        next(np);
        return Intfunc(np, s);
    } else {
        assert(toBoolfunc(s));
        next(np);
        return Boolfunc(np, s);
    }
}

void * Listfunc(Nuclei *np, char *s)
{
    bool car = _strcmp(s, "CAR");
    bool cdr = _strcmp(s, "CDR");
    bool cons = _strcmp(s, "CONS");
    assert(car || cdr || cons);
    #ifdef INTERP
        retVal *v1 = (retVal *) List(np);
        checkType(v1, LISP);
        retVal *v = retVal_init();
        lisp *l1 = v1->lp;
        if (car) {
            setRetvalLisp(v, lisp_car(l1));
        } else if (cdr) {
            setRetvalLisp(v, lisp_cdr(l1));
        } else {
            retVal *v2 = (retVal *) List(np);
            checkType(v2, LISP);
            setRetvalLisp(v, lisp_cons(np, l1, v2->lp));
        }
        trackRetval(np, v);
        return v;
    #else
        List(np);
        if (cons) {
            List(np);
        }
        return NULL;
    #endif
}

void * Intfunc(Nuclei *np, char *s)
{
    bool plus = _strcmp(s, "PLUS");
    bool len = _strcmp(s, "LENGTH");
    assert(plus || len);
    #ifdef INTERP
        retVal *v1 = (retVal *) List(np);
        checkType(v1, LISP);
        retVal *v = retVal_init();
        lisp *l1 = v1->lp;
        lisp *li;
        if (len) {
            li = lisp_atom(np, lisp_length(l1));
        } else {
            checkAtom(v1);
            atomtype a1 = lisp_getval(l1);
            retVal *v2 = (retVal *) List(np);
            checkAtom(v2);
            atomtype a2 = lisp_getval(v2->lp);
            li = lisp_atom(np, a1 + a2);
        }
        setRetvalLisp(v, li);
        trackRetval(np, v);
        return v;
    #else
        List(np);
        if (plus) {
            List(np);
        }
        return NULL;
    #endif
}

void * Boolfunc(Nuclei *np, char *s)
{
    bool l = _strcmp(s, "LESS");
    bool g = _strcmp(s, "GREATER");
    bool e = _strcmp(s, "EQUAL");
    assert(l || g || e);
    #ifdef INTERP
        retVal *v1 = (retVal *) List(np);
        checkAtom(v1);
        atomtype a1 = lisp_getval(v1->lp);
        retVal *v2 = (retVal *) List(np);
        checkAtom(v2);
        atomtype a2 = lisp_getval(v2->lp);
        bool result = false;
        if ((l && a1 < a2) || (g && a1 > a2) || (e && a1 == a2)) {
            result = true;
        }
        retVal *v = retVal_init();
        setRetvalBool(v, result);
        trackRetval(np, v);
        return v;
    #else
        List(np);
        List(np);
        return NULL;
    #endif 
}

void IOfunc(Nuclei *np, char *s)
{
    if (toSet(s)) {
        next(np);
        Set(np);
    } else {
        assert(toPrint(s));
        next(np);
        Print(np);
    }
}

void Set(Nuclei *np)
{
    char *s = getToken(np);
    checkError(isVar(s), "VAR");
    next(np);
    #ifdef INTERP
        int i = s[0] - 'A';
        (np->hasSet)[i] = true;
        (np->variables)[i] = (retVal *) List(np);
    #else
        List(np);
    #endif
}

void Print(Nuclei *np)
{
    char *s = getToken(np);
    if (isString(s)) {
        next(np);
        #ifdef INTERP
            char withoutquotes[MAXTOKENSIZE];
            int length = strlen(s);
            strcpy(withoutquotes, &(s[1]));
            withoutquotes[length - 2] = STREND;
            printf("%s\n", withoutquotes);
        #endif
    } else {
        #ifdef INTERP
            printList(np);
        #else
            List(np);
        #endif
    }
}

void If(Nuclei *np)
{
    next(np);
    checkBracket(np, OPENBRACKET);
    char *nextStr = getToken(np);
    next(np);
    #ifdef INTERP
        bool condition = Boolfunc(np, nextStr);
    #else
        Boolfunc(np, nextStr);
    #endif
    checkBracket(np, CLOSEBRACKET);
    checkBracket(np, OPENBRACKET);
    #ifdef INTERP
        if (condition) {
            Instrcts(np);
        } else {
            skip(np);
        }
    #else
        Instrcts(np);
    #endif
    checkBracket(np, OPENBRACKET);
    #ifdef INTERP
        if (condition) {
            skip(np);
        } else {
            Instrcts(np);
        }
    #else
        Instrcts(np);
    #endif
}

void Loop(Nuclei *np)
{
    next(np);
    checkBracket(np, OPENBRACKET);
    char *s = getToken(np);
    checkError(toBoolfunc(s), "BOOLFUNC");
    next(np);
    #ifdef INTERP
        int back = np->currToken;
        retVal *v = (retVal *) Boolfunc(np, s);
        checkType(v, BOOL);
        bool condition = v->b;
    #else
        Boolfunc(np, s);
    #endif
    checkBracket(np, CLOSEBRACKET);
    checkBracket(np, OPENBRACKET);
    #ifdef INTERP
        if (!condition) {
            skip(np);
            return;
        }
        int start = np->currToken;
        while (condition) {
            Instrcts(np);
            np->currToken = back;
            v = (retVal *) Boolfunc(np, s);
            checkType(v, BOOL);
            condition = v->b;
            np->currToken = start;
        }
        if (!condition) {
            np->currToken = start;
            skip(np);
            return;
        }
    #else
        Instrcts(np);
    #endif
}

void * List(Nuclei *np)
{
    char *s = getToken(np);
    if (isVar(s)) {
        next(np);
        #ifdef INTERP
            return findVar(np, s);
        #endif
    } else if (isLiteral(s)) {
        next(np);
        #ifdef INTERP
            return literalToLisp(np, s);
        #endif
    } else if (isNil(s)) {
        next(np);
        #ifdef INTERP
            retVal *v = retVal_init();
            setRetvalLisp(v, NULL);
            trackRetval(np, v);
            return v;
        #endif
    } else {
        checkBracket(np, OPENBRACKET);
        char *nextStr = getToken(np);
        checkError(toRetfunc(nextStr), "RETFUNC");
        #ifdef INTERP
            retVal *v = Retfunc(np, nextStr);
        #else
            Retfunc(np, nextStr);
        #endif
        checkBracket(np, CLOSEBRACKET);
        #ifdef INTERP
            return v;
        #endif
    }
    #ifndef INTERP
        return NULL;
    #endif
}

bool isVar(char *s)
{
    return strlen(s) == 1 && isupper(s[0]);
}

bool isNil(char *s)
{
    return _strcmp(s, "NIL");
}

bool isString(char *s)
{
    return isStrOrLiteral(s, DOUBLEQUOTE);
}

bool isLiteral(char *s)
{
    return isStrOrLiteral(s, SINGLEQUOTE);
}

bool isStrOrLiteral(char *s, char *quote)
{
    char q = quote[0];
    if (s[0] != q) {
        return false;
    }
    int length = strlen(s);
    if (length < 2 || s[length - 1] != q) {
        checkError(false, quote);
    }
    return true;
}

void * Literal(Nuclei *np, char *s)
{
    next(np);
    assert(isLiteral(s));
    #ifdef INTERP
        return literalToLisp(np, s);
    #else
        return NULL;
    #endif
}

bool toFunc(char *s)
{
    return toRetfunc(s) || toIOfunc(s) || toIf(s) || toLoop(s);
}

bool toRetfunc(char *s)
{
    return toListfunc(s) || toIntfunc(s) || toBoolfunc(s); 
}

bool toListfunc(char *s)
{
    return _strcmp(s, "CAR") || _strcmp(s, "CDR") || _strcmp(s, "CONS");
}

bool toIntfunc(char *s)
{
    return _strcmp(s, "PLUS") || _strcmp(s, "LENGTH");
}

bool toBoolfunc(char *s)
{
    return _strcmp(s, "LESS") || _strcmp(s, "GREATER") || _strcmp(s, "EQUAL");
}

bool toIOfunc(char *s)
{
    return toSet(s) || toPrint(s);
}

bool toSet(char *s)
{
    return _strcmp(s, "SET");
}

bool toPrint(char *s)
{
    return _strcmp(s, "PRINT");
}

bool toIf(char *s)
{
    return _strcmp(s, "IF");
}

bool toLoop(char *s)
{
    return _strcmp(s, "WHILE");
}

bool _strcmp(char *s1, char *s2)
{
    return strcmp(s1, s2) == 0;
}

void next(Nuclei *np)
{
   if (np->currToken < np->totalTokens) {
        np->currToken += 1;
   }
}

void checkBracket(Nuclei *np, char *bracket)
{
    char *s = getToken(np);
    checkError(_strcmp(s, bracket), bracket);
    next(np);
}

void checkError(bool expected, char *element)
{
    if (!expected) {
        fprintf(stderr, "Was expecting %s\n", element);
        exit(EXIT_FAILURE);
    }
}

char * getToken(Nuclei *np)
{
    return (np->tokens)[np->currToken];
}

Nuclei * nuclei_init(void)
{
    return ncalloc(1, sizeof(Nuclei));
}

void nuclei_free(Nuclei **np)
{
    if (!np) {
        return;
    }
    #ifdef INTERP
        if (*np != NULL) {
            free_lisps(*np);
            free_vals(*np);
        }
    #endif
    free(*np);
    *np = NULL;
}

#ifdef INTERP
    retVal * retVal_init(void)
    {
        return ncalloc(1, sizeof(retVal));
    }

    retVal * findVar(Nuclei *np, char *s)
    {
        assert(isVar(s));
        int i = s[0] - 'A';
        if (!(np->hasSet)[i]) {
            on_error("VAR is not set?");
        }
        return (np->variables)[i];
    }

    void trackRetval(Nuclei *np, retVal *v)
    {
        LList *original = np->val_generated;
        LList *new = ncalloc(1, sizeof(LList));
        new->latest = v;
        new->previous = original;
        np->val_generated = new;
        np->val_count += 1;
    }

    void free_vals(Nuclei *np)
    {
        while (np->val_generated != NULL) {
            _free(&((np->val_generated)->latest));
            LList *tmp = (np->val_generated)->previous;
            free(np->val_generated);
            np->val_generated = tmp;
            np->val_count -= 1;
        }
        /* printf("%i\n", np->val_count); */
        assert(np->val_count == 0);
    }

    void _free(void **v)
    {
        if (!v) {
            return;
        } else if (!*v) {
            *v = NULL;
            return;
        }
        free(*v);
        *v = NULL;
    }

    void trackLisp(Nuclei *np, lisp *l)
    {
        LList *original = np->lisp_generated;
        LList *new = ncalloc(1, sizeof(LList));
        new->latest = l;
        new->previous = original;
        np->lisp_generated = new;
        np->lisp_count += 1;
    }

    void free_lisps(Nuclei *np)
    {
        while (np->lisp_generated != NULL) {
            _free(&((np->lisp_generated)->latest));
            LList *tmp = (np->lisp_generated)->previous;
            free(np->lisp_generated);
            np->lisp_generated = tmp;
            np->lisp_count -= 1;
        }
        assert(np->lisp_count == 0);
    }

    void checkType(retVal *v, char *t)
    {
        assert(_strcmp(t, LISP) || _strcmp(t, BOOL));
        if (!_strcmp(v->type, t)) {
            on_error("Wrong type of variable?");
        }
    }

    void checkAtom(retVal *v)
    {
        checkType(v, LISP);
        checkError(lisp_isatomic(v->lp), "Atomic LIST");
    }

    void setRetvalLisp(retVal *v, lisp *li)
    {
        strcpy(v->type, LISP);
        v->lp = li;
    }

    void setRetvalBool(retVal *v, bool boo)
    {
        strcpy(v->type, BOOL);
        v->b = boo;
    }

    void skip(Nuclei* np)
    {
        int open = 0;
        char *s = getToken(np);
        while (open > 0 || !_strcmp(s, CLOSEBRACKET)) {
            if (_strcmp(s, OPENBRACKET)) {
                open += 1;
            } else if (_strcmp(s, CLOSEBRACKET)) {
                open -= 1;
            }
            if (open != 0 && np->currToken == np->totalTokens) {
                checkError(false, CLOSEBRACKET);
            }
            next(np);
            s = getToken(np);
        }
        assert(open == 0 && _strcmp(getToken(np), CLOSEBRACKET));
        next(np);
    }

    void printList(Nuclei *np)
    {
        retVal *v = (retVal *) List(np);
        char *t = v->type;
        if (_strcmp(t, LISP)) {
            char s[MAXTOKENSIZE];
            lisp_tostring(v->lp, s);
            printf("%s\n", s);
        } else {
            assert(_strcmp(t, BOOL));
            if (v->b) {
                printf("true\n");
            } else {
                printf("false\n");
            }
        }
    }

    void * literalToLisp(Nuclei *np, char *s)
    {
        assert(isLiteral(s));
        int length = strlen(s);
        char withoutquotes[MAXTOKENSIZE];
        strcpy(withoutquotes, &(s[1]));
        withoutquotes[length - 2] = STREND;
        retVal *v = retVal_init();
        setRetvalLisp(v, lisp_fromstring(np, withoutquotes));
        trackRetval(np, v);
        return v;
    }

#endif
