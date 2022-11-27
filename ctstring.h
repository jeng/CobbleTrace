#ifndef __CT_STRING__
#define __CT_STRING__
struct String {
    int size;
    char *data;
};

extern void StringToCharPtr(String s, char *p, int max);
extern String CharPtrToString(const char *p);
extern bool IsStringEqual(String *a, String *b);
extern bool IsStringEqual(String *a, const char *b);
#endif