#ifndef __CT_STRING__
#define __CT_STRING__
struct string_t {
    int size;
    char *data;
};

extern void StringToCharPtr(string_t s, char *p, int max);
extern string_t CharPtrToString(const char *p);
extern bool IsStringEqual(string_t *a, string_t *b);
extern bool IsStringEqual(string_t *a, const char *b);
#endif