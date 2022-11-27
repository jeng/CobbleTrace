#include "ctstring.h"

String
CharPtrToString(const char *p){
    String result;
    result.size = 0;
    result.data = (char *)p;
    while(*p != '\0'){
        result.size++;
        p++;
    }
    return result;
}

void
StringToCharPtr(String s, char *p, int max){
    for(int i = 0; i < max && i < s.size; i++){
        p[i] = s.data[i];
    }
    if (s.size < max)
        p[s.size] = '\0';
    else
        p[max - 1] = '\0';
}

bool
IsStringEqual(String *a, String *b){
    if (a->size != b->size)
        return false;
    for(int i = 0; i < a->size; i++) {
        if (a->data[i] != b->data[i])
            return false;
    }
    return true;
}

bool
IsStringEqual(String *a, const char *b){
    String sB = CharPtrToString(b);
    return IsStringEqual(a, &sB);
}