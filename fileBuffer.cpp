#include "fileBuffer.h"

void
GetNextBuffer(FileBuffer *fb){
    fseek(fb->file, fb->nextOffset, SEEK_SET);
    int bytesRead = fread(fb->buffer, sizeof(char), FB_BUFFER_SZ, fb->file);
    fb->prevOffset = fb->nextOffset;
    fb->nextOffset = ftell(fb->file);
    fb->index = 0;
}

void
GetPrevBuffer(FileBuffer *fb){
    fb->prevOffset -= FB_BUFFER_SZ;
    fseek(fb->file, fb->prevOffset, SEEK_SET);
    int bytesRead = fread(fb->buffer, sizeof(char), FB_BUFFER_SZ, fb->file);
    fb->nextOffset = ftell(fb->file);
    fb->index = FB_BUFFER_SZ - 1;
 }

bool
IsEOF(FileBuffer *fb){
    return (fb->index + fb->prevOffset > fb->size);
}

void
NextChar(FileBuffer *fb){
    fb->index++;
    if (fb->index >= FB_BUFFER_SZ){
        if (!IsEOF(fb)){
            GetNextBuffer(fb);
        }
    }
}

void
SkipSpace(FileBuffer *fb){
    while(!IsEOF(fb)){
        if (fb->buffer[fb->index] != '\n' && 
            fb->buffer[fb->index] != '\r' && 
            fb->buffer[fb->index] != ' ' && 
            fb->buffer[fb->index] != '\t'){            
            return;
        }
        NextChar(fb);
    }
}

void
SkipLine(FileBuffer *fb){
    while(!IsEOF(fb) && fb->buffer[fb->index] != '\n' && fb->buffer[fb->index] != '\r'){
        NextChar(fb);
    }
    SkipSpace(fb);
}

char
GetToken(FileBuffer *fb){
    if (IsEOF(fb)){
        return 0;
    }
    SkipSpace(fb);
    char result = fb->buffer[fb->index];
    NextChar(fb);
    return result;
}

void
PushToken(FileBuffer *fb){
    if (fb->index > 0)
        fb->index--;
    else {
        GetPrevBuffer(fb);
    }        
}

void
GetString(FileBuffer *fb, String *result, int maxSize){
    result->size = 0;

    SkipSpace(fb);

    if (IsEOF(fb))
        return;

    assert(fb->buffer[fb->index] == '"');

    NextChar(fb);
    
    if (IsEOF(fb))
        return;

    //I will need to use a string pool for this or have the string buffer passed in
    int i = 0;
    while(i < maxSize && !IsEOF(fb) && fb->buffer[fb->index] != '"'){
        result->data[i++] = fb->buffer[fb->index];
        result->size++;
        NextChar(fb);
    }

    NextChar(fb);
}

bool
GetBoolean(FileBuffer *fb){
    bool result;
    String s;
    char d[10] = {0};
    s.size = 0;
    s.data = (char*)&d;
    SkipSpace(fb);

    while(!IsEOF(fb) && s.size < 5 && 'a' <= fb->buffer[fb->index] && fb->buffer[fb->index] <= 'z'){
        s.data[s.size++] = fb->buffer[fb->index];
        NextChar(fb);
    }

    if (IsStringEqual(&s, "true"))
        return true;
    else if (IsStringEqual(&s, "false"))
        return false;
    else
        assert(false);

    return false;
}

void
AssertNextToken(FileBuffer *fb, char c){
    char x = GetToken(fb);
    if (c != x){
        char s[100];
        sprintf(s, "expected %c got %c\n", c, x);
        SDL_Log("%s", s);
    }
    assert(c == x);
}

float
GetNumber(FileBuffer *fb){
    char c = GetToken(fb);
    float n = 1;
    float result = 0;
    bool decimal = false;
    float d = 10;

    if (c == '-'){
        n = -1;
    } else {
        PushToken(fb);
    }
    
    while(!IsEOF(fb)){
        c =  fb->buffer[fb->index];
        if ('0' <= c && c <= '9'){
            if (decimal){
                float x = c - '0';
                x = x/d;
                result += x;
                d *= 10;
                
            } else {
                result = result * 10;
                result += c - '0';
            }
        } else if (c == '.'){            
            assert(!decimal);
            decimal = true;
        } else {
            break;
        }
        NextChar(fb);
    }

    return n * result;    
}

v3_t
GetV3(FileBuffer *fb){
    float f;
    v3_t v;
    AssertNextToken(fb, '[');
    v.x = GetNumber(fb);
    AssertNextToken(fb, ',');
    v.y = GetNumber(fb);
    AssertNextToken(fb, ',');
    v.z = GetNumber(fb);
    AssertNextToken(fb, ']');
    return v;
}

v3_t
GetV3Raw(FileBuffer *fb){
    float f;
    v3_t v;
    SkipSpace(fb);
    v.x = GetNumber(fb);
    SkipSpace(fb);
    v.y = GetNumber(fb);
    SkipSpace(fb);
    v.z = GetNumber(fb);
    return v;
}


void OpenFileBuffer(FileBuffer *fb, char *filename){
    fb->file = fopen(filename, "rb");

    assert(fb->file != NULL);
    fseek(fb->file, 0, SEEK_END);
    fb->size = ftell(fb->file);
    fseek(fb->file, 0, SEEK_SET);

    int bytesRead = fread(fb->buffer, sizeof(char), FB_BUFFER_SZ, fb->file);
    fb->prevOffset = 0;
    fb->nextOffset = ftell(fb->file);
    fb->index = 0;
 }

void CloseFileBuffer(FileBuffer *fb){
    fclose(fb->file);
}

