#ifndef __FILE_BUFFER_H__
#define __FILE_BUFFER_H__

#include <stdio.h>
#include "ctstring.h"
#include "mymath.h"
#include "SDL.h"

#define FB_BUFFER_SZ (4096)
#define FB_LINE_SZ (1024)

struct FileBuffer{
    FILE *file;
    size_t size;
    size_t prevOffset;
    size_t nextOffset;
    int index;
    char buffer[FB_BUFFER_SZ];
    char line[FB_LINE_SZ];
};

extern void OpenFileBuffer(FileBuffer *fb, char *filename);
extern void SkipSpace(FileBuffer *fb);
extern char GetToken(FileBuffer *fb);
extern void PushToken(FileBuffer *fb);
extern void GetString(FileBuffer *fb, String *result, int maxSize);
//extern String GetString(FileBuffer *fb);
extern bool GetBoolean(FileBuffer *fb);
extern void AssertNextToken(FileBuffer *fb, char c);
extern float GetNumber(FileBuffer *fb);
extern v3_t GetV3(FileBuffer *fb);
extern v3_t GetV3Raw(FileBuffer *fb);
extern void NextToken(FileBuffer *fb); 
extern void CloseFileBuffer(FileBuffer *fb);
extern bool IsEOF(FileBuffer *fb);
extern void SkipLine(FileBuffer *fb);
 
#endif //__FILE_BUFFER_H__
