#include "fileBuffer.h"

void
GetNextBuffer(filebuffer_t *fb){
    fseek(fb->file, fb->nextOffset, SEEK_SET);
    int bytesRead = fread(fb->buffer, sizeof(char), FB_BUFFER_SZ, fb->file);
    fb->prevOffset = fb->nextOffset;
    fb->nextOffset = ftell(fb->file);
    fb->index = 0;
}

void
GetPrevBuffer(filebuffer_t *fb){
    fb->prevOffset -= FB_BUFFER_SZ;
    fseek(fb->file, fb->prevOffset, SEEK_SET);
    int bytesRead = fread(fb->buffer, sizeof(char), FB_BUFFER_SZ, fb->file);
    fb->nextOffset = ftell(fb->file);
    fb->index = FB_BUFFER_SZ - 1;
 }

bool
IsEOF(filebuffer_t *fb){
    return (fb->index + fb->prevOffset > fb->size);
}

void
NextChar(filebuffer_t *fb){
    fb->index++;
    if (fb->index >= FB_BUFFER_SZ){
        if (!IsEOF(fb)){
            GetNextBuffer(fb);
        }
    }
}

bool
IsSpace(filebuffer_t *fb){
    return 
        (fb->buffer[fb->index] == '\n' || 
        fb->buffer[fb->index] == '\r' || 
        fb->buffer[fb->index] == ' '  || 
        fb->buffer[fb->index] == '\t');            
}

void
SkipSpace(filebuffer_t *fb){
    while(!IsEOF(fb) && IsSpace(fb)){
        NextChar(fb);
    }
}

void
SkipLine(filebuffer_t *fb){
    while(!IsEOF(fb) && fb->buffer[fb->index] != '\n' && fb->buffer[fb->index] != '\r'){
        NextChar(fb);
    }
    SkipSpace(fb);
}

char
GetToken(filebuffer_t *fb){
    if (IsEOF(fb)){
        return 0;
    }
    SkipSpace(fb);
    char result = fb->buffer[fb->index];
    NextChar(fb);
    return result;
}

void
PushToken(filebuffer_t *fb){
    if (fb->index > 0)
        fb->index--;
    else {
        GetPrevBuffer(fb);
    }        
}

void
GetString(filebuffer_t *fb, string_t *result, int maxSize){
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

void
GetStringRaw(filebuffer_t *fb, string_t *result, int maxSize){
    result->size = 0;

    SkipSpace(fb);

    if (IsEOF(fb))
        return;

    //I will need to use a string pool for this or have the string buffer passed in
    int i = 0;
    while(i < maxSize && !IsEOF(fb) && !IsSpace(fb)){
        result->data[i++] = fb->buffer[fb->index];
        result->size++;
        NextChar(fb);
    }
}

bool
GetBoolean(filebuffer_t *fb){
    bool result;
    string_t s;
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
AssertNextToken(filebuffer_t *fb, char c){
    char x = GetToken(fb);
    if (c != x){
        char s[100];
        sprintf(s, "expected %c got %c\n", c, x);
        SDL_Log("%s", s);
    }
    assert(c == x);
}

float
GetNumber(filebuffer_t *fb){
    char c = GetToken(fb);
    float n = 1;
    float result = 0;
    bool decimal = false;
    bool scientific = false;
    int exponent = 0;
    int esign = 1;
    float d = 10;

    if (c == '-'){
        n = -1;
    } else {
        PushToken(fb);
    }
    
    while(!IsEOF(fb)){
        c =  fb->buffer[fb->index];
        if ('0' <= c && c <= '9'){
            if (scientific){
                exponent = exponent * 10;
                exponent += c - '0'; 
            } else if (decimal){
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
        } else if (c == 'e'){
            assert(!scientific);
            scientific = true;
        } else if (c == '-' && scientific){
            esign = -1;
        } else {
            break;
        }
        NextChar(fb);
    }

    return (n * result) * pow(10, esign * exponent);    
}

v3_t
GetV3(filebuffer_t *fb){
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
GetV3Raw(filebuffer_t *fb){
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

void OpenFileBuffer(filebuffer_t *fb, char *filename){
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

 void OpenFileBuffer(filebuffer_t *fb, string_t filename){
    char *s;
    int filenameSize = filename.size + 1;
    s = (char *)malloc(filenameSize * sizeof(char));
    assert(s != NULL);
    StringToCharPtr(filename, s, filenameSize);
    OpenFileBuffer(fb, s);
    free(s);
 }

void CloseFileBuffer(filebuffer_t *fb){
    fclose(fb->file);
}

