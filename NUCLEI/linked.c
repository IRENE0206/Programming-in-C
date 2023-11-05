#include "nuclei.h"

#ifdef INTERP
#define LISPIMPL "Linked"
#define FORMATSTR "%i"
#define SPACE " "
#define NEGATIVE '-'

lisp * node_init(void)
{
    node *l = (node *) ncalloc(1, sizeof(node));
    l->head = NULL;
    l->remainder = NULL;
    return (lisp *) l;
}

lisp * lisp_atom(Nuclei *np, const atomtype a)
{
    lisp *l = node_init();
    ((node *) l)->val = a;
    trackLisp(np, l);
    return l;
}

bool lisp_isatomic(const lisp *l)
{
    node *lp = (node *) l;
    if (lp && !(lp->head) && !(lp->remainder)) {
        return true;
    }
    return false;
}

lisp * lisp_cons(Nuclei *np, const lisp *l1,  const lisp *l2)
{
    lisp *l = node_init();
    ((node *) l)->head = (node *) l1;
    ((node *) l)->remainder = (node *) l2;
    trackLisp(np, l);
    return l;
}

lisp * lisp_car(const lisp *l)
{
    assert(l);
    return (lisp *) (((node *) l)->head);
}

lisp * lisp_cdr(const lisp *l)
{
    assert(l);
    return (lisp *) (((node *) l)->remainder);
}

atomtype lisp_getval(const lisp *l)
{
    assert(l);
    return ((node *) l)->val;
}

void length_helper(const lisp *l, int *length)
{
    if (!l || lisp_isatomic(l)) {
        return;
    }
    *length += 1;
    length_helper(lisp_cdr(l), length);
}

int lisp_length(const lisp *l)
{
    int length = 0;
    length_helper(l, &length);
    return length;
}

void lisp_tostring(const lisp *l, char *str)
{
    str[0] = STREND;
    if (lisp_isatomic(l)) {
        sprintf(str, FORMATSTR, lisp_getval(l));
        return;
    }
    tostring_helper(l, str, true);
}

void tostring_helper(const lisp *l, char *str, bool brackets)
{
    if (brackets) {
        strcat(str, OPENBRACKET);
    }
    if (!l) {
        strcat(str, CLOSEBRACKET);
        return;
    }
    node *lp = (node *) l;
    char tmp[BIG];
    if (lisp_isatomic(l)) {
        sprintf(tmp, FORMATSTR, lp->val);
        strcat(str, tmp);
        return;
    }
    lisp *h = lisp_car(l);
    bool is_atom = lisp_isatomic(h);
    tostring_helper(h, str, !is_atom);
    lisp *r = lisp_cdr(l);
    if (r) {
        strcat(str, SPACE);
        tostring_helper(r, str, false);
    }
    if (brackets) {
        strcat(str, CLOSEBRACKET);
    }
}

void free_lisp(lisp **l)
{
    if (!l) {
        return;
    } else if (!*l) {
        *l = NULL;
        return;
    }
    free(*l);
    *l = NULL;
}

lisp * lisp_fromstring(Nuclei *np, const char *str)
{
    int index = 0;
    char c = str[index];
    if (isdigit(c) || c == NEGATIVE) {
        atomtype a;
        assert(sscanf(str, FORMATSTR, &a) == 1);
        return lisp_atom(np, a);
    }
    int length = strlen(str);
    return fromstr_helper(np, str, length, &index);
}

void str_to_int(const char *str, int *start, int *end, atomtype *result)
{
    char c = str[*start];
    assert(isdigit(c) || c == NEGATIVE);
    if (*end > *start && !isdigit(str[*end])) {
        assert(sscanf(&(str[*start]), FORMATSTR, result) == 1);
        return;
    }
    *end += 1;
    str_to_int(str, start, end, result);
}

lisp * fromstr_helper(Nuclei *np, const char *str, int length, int *index)
{
    check_valid(*index < length);
    char curr = str[*index];
    char blank = SPACE[0];
    if (curr == blank) {
        *index += 1;
        return fromstr_helper(np, str, length, index);
    }
    char close = CLOSEBRACKET[0];
    if (curr == close) {
        if (*index < length - 1) {
            *index += 1;
        }
        return NULL;
    }
    if (curr == NEGATIVE || isdigit(curr)) {
        atomtype a;
        int start = *index, end = *index;
        str_to_int(str, &start, &end, &a);
        *index = end;
        lisp *first = lisp_atom(np, a);
        lisp *rest = fromstr_helper(np, str, length, index);
        return lisp_cons(np, first, rest);
    }
    char open = OPENBRACKET[0];
    check_valid(curr == open);
    char prev = STREND;
    if (*index > 0) {
        prev = str[*index - 1];
    }
    check_valid(*index < length - 1);
    *index += 1;
    char next = str[*index];
    if (((*index > 0) && (isdigit(prev) || prev == blank)) || (next == open)) {
        lisp *first = fromstr_helper(np, str, length, index);
        lisp *left = fromstr_helper(np, str, length, index);
        return lisp_cons(np, first, left);
    }
    if (next == close) {
        return NULL;
    }
    return fromstr_helper(np, str, length, index);
}

void check_valid(bool expected)
{
    if (!expected) {
        on_error("LITERAL is not valid list");
    }
}
#endif
