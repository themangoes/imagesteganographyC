// header file to include function declarations, macros and stuff

#ifndef HELPERS_H_
#define HELPERS_H_

// naming the unsigned 8-bit integer type
// (included in ctype.h) to BYTE
typedef __uint8_t BYTE;

// macros for everything
#define BMP 1
#define JPG 2
#define PNG 3
#define UNSUPPORTEDTYPE -1
#define BITMAPHEADERSIZE 14
#define BMPSIGNATUREBYTES 0x424D
#define JPGSIGNATUREBYTES 0xFFD8
#define PNGSIGNATUREBYTES 0x8950
#define BYTESIZE 8
#define SIGNATUREBYTESIZE 2


// function declarations

// functions that are used only in writemessage.c
char* get_string(char* prompt);
int checkFileType(FILE* in, FILE* out);
int copyHeaderForBMP(FILE* in, FILE* out);
void readNWriteFor(int numberOfBytes, FILE* in, FILE* out);
void editBufferToStoreChar(BYTE* buffer, int ch);
void changeLSBOf(BYTE* byte, int toWhat);

// functions that are used only in readmessage.c
int readCharFromLSBAndPrint(BYTE* buffer, char* passkey);
int readHeaderForBMP(FILE* file);

void encrypt(char* text, char* passkey);
char decryptChar(char c, char* passkey);
int hash(char* passkey);

#endif
