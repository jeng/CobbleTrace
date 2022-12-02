#ifndef __FILE_BUFFER_H__
#define __FILE_BUFFER_H__

#include <stdio.h>
#include "ctstring.h"
#include "mymath.h"
#include "SDL.h"

#define FB_BUFFER_SZ (4096)
#define FB_LINE_SZ (1024)

struct filebuffer_t{
    FILE *file;
    size_t size;
    size_t prevOffset;
    size_t nextOffset;
    int index;
    char buffer[FB_BUFFER_SZ];
    char line[FB_LINE_SZ];
};

extern void OpenFileBuffer(filebuffer_t *fb, char *filename);
extern void OpenFileBuffer(filebuffer_t *fb, string_t filename);
extern void SkipSpace(filebuffer_t *fb);
extern char GetToken(filebuffer_t *fb);
extern void PushToken(filebuffer_t *fb);
extern void GetString(filebuffer_t *fb, string_t *result, int maxSize);
extern void GetStringRaw(filebuffer_t *fb, string_t *result, int maxSize);
//extern string_t GetString(filebuffer_t *fb);
extern bool GetBoolean(filebuffer_t *fb);
extern void AssertNextToken(filebuffer_t *fb, char c);
extern float GetNumber(filebuffer_t *fb);
extern v3_t GetV3(filebuffer_t *fb);
extern v3_t GetV3Raw(filebuffer_t *fb);
extern void NextToken(filebuffer_t *fb); 
extern void CloseFileBuffer(filebuffer_t *fb);
extern bool IsEOF(filebuffer_t *fb);
extern void SkipLine(filebuffer_t *fb);
 
#endif //__FILE_BUFFER_H__
