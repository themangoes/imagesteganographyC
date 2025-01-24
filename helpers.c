// this file has helper functions required for writemessage.c
// and readmessage.c to work.

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"


// function to get a string from the user of any size.
// this works by reallocating memory for the string every
// time a character is typed in. If the user hits "enter"
// i.e, inputs a new line character ('\n') then this function
// stops taking inputs and returns the string (including the new
// line character).
char* get_string(char* prompt)
{
    // prints the prompt given.
    printf("%s", prompt);

    // character to store the input.
    char c;

    // string that stores all the inputted characters.
    char* string = malloc(sizeof(char));

    // index to keep track of number of characters inputted.
    int i = 0;

    // keeps taking inputs until new line character ('\n') is reached.
    do{
        // inputs a char into c.
        scanf("%c", &c);

        // reallocates memory to make space for the newly
        // inputted character.
        string = realloc(string, i + 1);

        // add the character to the string.
        string[i++] = c;
    } while (c != '\n' && c != '\r' && c != EOF);

    // add the terminating character (NUL or '\0') to indicate the
    // end of string.
    string[i] = '\0';

    // return the inputted string.
    return string;
}


// this function checks if the file type is either
// bmp, jpg or png (or unsupported).
int checkFileType(FILE* in, FILE* out)
{
    // buffer to store the first 2 bytes in the input file
    // that are the "signature" bytes (unique bytes that are
    // different for every file type).
    BYTE buffer[SIGNATUREBYTESIZE];

    // set the cursor to the start of the file for both
    // input and output files (if output file is given).
    fseek(in, 0, SEEK_SET);

    if (out != NULL)
        fseek(out, 0, SEEK_SET);

    // read the bytes into the buffer.
    fread(buffer, SIGNATUREBYTESIZE, 1, in);
    // write into the output file if given
    if (out != NULL)
        fwrite(buffer, SIGNATUREBYTESIZE, 1, out);

    // combine the two 8-bit numbers into a single
    // 16-bit number
    int signatureBytes = buffer[0] << 8 | buffer[1];

    // check which type of file it is and return the type.
    if (signatureBytes == BMPSIGNATUREBYTES)
        return BMP;
    else if (signatureBytes == JPGSIGNATUREBYTES)
        return JPG;
    else if (signatureBytes == PNGSIGNATUREBYTES)
        return PNG;
    else
        return UNSUPPORTEDTYPE;
}


// function to copy the header data of bmp file
// from in to out.
int copyHeaderForBMP(FILE* in, FILE* out)
{
    BYTE buffer[BITMAPHEADERSIZE - SIGNATUREBYTESIZE];
    fread(buffer, BITMAPHEADERSIZE - SIGNATUREBYTESIZE, 1, in);
    fwrite(buffer, BITMAPHEADERSIZE - SIGNATUREBYTESIZE, 1, out);

    // header data contains the position of the start of pixel array
    // calculate it, store it and return it.
    int pixelArrayOffset = (((buffer[10] << 8) | buffer[9]) << 8) | buffer[8];
    return pixelArrayOffset;
}


// function to read from in and write to out for the specified
// number of bytes. If the number of bytes specified is -1,
// read and write all the data from in to out.
void readNWriteFor(int numberOfBytes, FILE* in, FILE* out)
{
    if (numberOfBytes == -1)
    {
        BYTE buffer[BYTESIZE];
        while(fread(buffer, BYTESIZE, 1, in) != 0)
            fwrite(buffer, BYTESIZE, 1, out);

        return;
    }
    BYTE buffer[numberOfBytes];
    fread(buffer, numberOfBytes, 1, in);
    fwrite(buffer, numberOfBytes, 1, out);
}

// this function takes in a buffer of data and an int
// (which can also be a char). It edits the LSB of the
// bytes of data in the buffer to store the int/char.
void editBufferToStoreChar(BYTE* buffer, int ch)
{
    for (int i = 0; i < 8; i++)
    {
        if (ch % 2 == 1)
        {
            changeLSBOf(&buffer[i], 1);
        }
        else
            changeLSBOf(&buffer[i], 0);

        ch /= 2;
    }
}


// this function changes the LSB of the given BYTE
// of data to the specified bit.
void changeLSBOf(BYTE* byte, int toWhat)
{
    if ((int)(*byte) % 2 == toWhat)
        return;

    else
    {
        if (toWhat == 1)
            *byte = ((int)*byte) + 1;
        else
            *byte = ((int)*byte) - 1;
    }
}


// this function reads the LSB of each byte in the
// buffer, calculates it into a single integer, and
// prints it as a char.
int readCharFromLSBAndPrint(BYTE* buffer, char* passkey)
{
    int ch = 0;
    for (int i = 0; i < BYTESIZE; i++)
    {
        ch += (buffer[i] % 2) * pow(2, i);
    }

    if (passkey != NULL && ch != 0)
        ch = decryptChar(ch, passkey);

    printf("%c", ch);

    // if the buffer contained the special sequence to indicate the end of string
    // i.e, 0000 0000 or just 0, then return 1 to signify that all of the text
    // was printed.
    if (ch == 0)
        return 1;

    // if end of text not reached, return 0.
    return 0;
}


// this function reads the header of a bmp file and returns the pixel
// array position of the said file that is stored in the header.
int readHeaderForBMP(FILE* file)
{
    BYTE buffer[BITMAPHEADERSIZE - SIGNATUREBYTESIZE];
    fread(buffer, 1, BITMAPHEADERSIZE - SIGNATUREBYTESIZE, file);

    int pixelArrayOffset = (((buffer[10] << 8) | buffer[9]) << 8) | buffer[8];
    return pixelArrayOffset;
}

// this function applies a simple encryption algorithm on the
// given text.
void encrypt(char* text, char* passkey)
{
    int hashNum = hash(passkey),  len = strlen(text);

    for (int i = 0; i < len; i++)
    {
        text[i] = (text[i] - hashNum);
    }
    text[len - 1] = '\n';
}

// decrypts the char passed.
char decryptChar(char c, char* passkey)
{
    if (c == '\n')
        return c;

    int hashNum = hash(passkey);
    return (c + hashNum);
}

// generates a simple hash for given passkey.
int hash(char* passkey)
{
    int hashNum = 0;

    for (int i = 0, len = strlen(passkey); i < len; i++)
    {
        hashNum += passkey[i];
    }

    return hashNum % 26;
}
