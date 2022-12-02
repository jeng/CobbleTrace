#include "ctstring.h"

string_t
CharPtrToString(const char *p){
    string_t result;
    result.size = 0;
    result.data = (char *)p;
    while(*p != '\0'){
        result.size++;
        p++;
    }
    return result;
}

void
StringToCharPtr(string_t s, char *p, int max){
    for(int i = 0; i < max && i < s.size; i++){
        p[i] = s.data[i];
    }
    if (s.size < max)
        p[s.size] = '\0';
    else
        p[max - 1] = '\0';
}

bool
IsStringEqual(string_t *a, string_t *b){
    if (a->size != b->size)
        return false;
    for(int i = 0; i < a->size; i++) {
        if (a->data[i] != b->data[i])
            return false;
    }
    return true;
}

bool
IsStringEqual(string_t *a, const char *b){
    string_t sB = CharPtrToString(b);
    return IsStringEqual(a, &sB);
}