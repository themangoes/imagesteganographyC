// ---------------------------------------------------------------------------------------------
// this program executes image steganography for the following types: BMP, JPG, PNG.
// steganography for BMP is done using LSB method.
// steganography for JPG and PNG is done by storing the secret data after the end of file.
// ---------------------------------------------------------------------------------------------
// LSB method is not used for JPG and PNG files as the pixel data is not stored in a raw form
// but stored as compressed data. So to edit the pixel data, the image must be uncompressed,
// then stored and then compressed back again. And a similar process must be undergone to read
// the data as well. If this compressed data is edited without proper knowledge, it may corrupt
// the image. Hence the data is just stored after a special sequence of bytes that JPG and PNG
// use to signify the end of image. PNG and JPG parsers stop reading after this EOF sequence of
// bytes have been reached so the message is not visible while viewing the image normally.
//
// BMP format stores the pixel data without compressing it in raw. Hence it is easy to edit
// the pixel data without corrupting the image.
// ---------------------------------------------------------------------------------------------
// go through helpers.c if you want to know how the user defined functions work.

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"


// argc is the number of command line arguments given.
// argv is an array of strings containing all the arguments.
int main(int argc, char* argv[])
{
    // if the number of arguments is not 3, i.e, ONLY an input image
    // AND ONLY an output image path is not provided, exit the program
    // with an error code -1.
    if (argc != 3)
    {
        printf("Incorrect usage.\nCorrect usage: ./writemessage <inputimagepath> <outputimagepath>\n");
        return -1;
    }

    // store the input image and output image paths in their
    // own separate strings.
    char* inputImagePath = argv[1];
    char* outputImagePath = argv[2];

    // open the input image, if the input image path is
    // not valid exit with error code 1.
    FILE* inimage = fopen(inputImagePath, "rb");
    if (inimage == NULL)
    {
        printf("Invalid input image path.\n");
        return 1;
    }

    // open the output image (or create it if it doesn't exist yet),
    // if the input image path is not valid exit with error code 2.
    FILE* outimage = fopen(outputImagePath, "w");
    if (outimage == NULL)
    {
        printf("Could not create output image.\n");
        return 2;
    }

    // check if the file type is supported (only BMP, PNG or JPEG).
    // if file type unsupported, exit with error code 3.
    int fileType = checkFileType(inimage, outimage);
    if (fileType == UNSUPPORTEDTYPE)
    {
        printf("Unsupported file type.\nSupported file types are BMP, JPG, PNG.\n");
        return 3;
    }

    // input the text to be secretly stored in the output image.
    char* hiddenText = get_string("Enter the text:\n");
    if (hiddenText == NULL)
    {
        // the get_string() function can give back a NULL value
        // if there is not enough space to store the text string
        // in that case exit with error code 4.
        printf("Something went wrong...\n");
        return 4;
    }
    // store the length of the text string in textlen.
    int textlen = strlen(hiddenText);

    // variable to keep track if the message has been stored.
    // 0 -> false (message not stored).
    // 1 -> true (message stored).
    int messageStored = 0;

    // execute operations according to file type.

    // operations to be done if image type is BMP:
    if (fileType == BMP)
    {
        // copyHeaderForBMP() copies the header to the output image and
        // returns the value of where the pixel array, i.e, all the RGB
        // values of the image starts.
        int pixelArrayOffset = copyHeaderForBMP(inimage, outimage);

        // copy all data from input to output image till the pixel array starts.
        // i.e, copy all the metadata into the output image.
        readNWriteFor(pixelArrayOffset - BITMAPHEADERSIZE, inimage, outimage);

        // a buffer (memory to store temporary data) to store one 8 BYTEs of data.
        BYTE buffer[BYTESIZE];

        // index to keep track of how much (or how many characters) of the
        // inputted text string has been stored in the image.
        int index = 0;

        // keep reading data into buffer from the input image until the end of
        // file (or until the message is being stored).
        while (fread(buffer, 1, BYTESIZE, inimage) != 0)
        {
            // if the index has reached the end of the inputted text
            // i.e index is equal to the text length then store a
            // special sequence of bytes (0000 0000) to indicate that
            // it is the end of the secret string.
            // this is needed so that the program knows when the complete
            // text has been read while reading it.
            if (index >= textlen)
            {
                editBufferToStoreChar(buffer, 0);
                // set messageStored to true (1).
                messageStored = 1;
            }

            // if message has not been stored, i.e,
            // messageStored -> false (0), then continue to add
            // characters from the input text to the image.
            if (messageStored == 0)
            {
                // put the character of the text at the current index in ch.
                int ch = hiddenText[index];

                // edits the buffer (8 bits of data read from the input image)
                // to store the current character from the string.
                editBufferToStoreChar(buffer, ch);
                index++;
            }

            // write the modified data (buffer) from the input image into
            // the output image.
            fwrite(buffer, 1, BYTESIZE, outimage);
        }
        // end of operations for bmp file.
    }
    // operations to be done if image type is JPG:
    else if (fileType == JPG)
    {
        // copies all the data from the input image to output image
        // up to (but not including) the end of file sequence.
        readNWriteFor(-1, inimage, outimage);

        // The two bytes in a JPG file that signify the end of file.
        BYTE EOFSequence[] = {0xFF, 0xD9};

        // write the end of file sequence into the output image.
        fwrite(EOFSequence, 2, 1, outimage);

        // write the inputted text into the output image after
        // the end of file bytes.
        fwrite(hiddenText, textlen, 1, outimage);

        // message has been stored.
        messageStored = 1;

        // end of operations for jpg file.
    }
    // operations to be done if image type is PNG:
    else if (fileType == PNG)
    {
        // a buffer to store ONE BYTE.
        BYTE buffer[1];

        // this is the sequence of bytes that signify the end of file in
        // a PNG file.
        int EOFSequence[] = {0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82};

        // number of bytes in the EOF sequence is 8.
        int EOFSequenceLen = 8;

        // index to keep track of the byte in the EOF sequence that has been
        // found in the file.
        int index = 0;

        // keep reading the input file until there is nothing to read
        // (until the EOF sequence is reached).
        while (fread(buffer, 1, 1, inimage) != 0)
        {
            // write whatever byte was read into the output image.
            fwrite(buffer, 1, 1, outimage);

            // if the current byte is following the EOF sequence
            // increment the index to check for the next byte in the
            // sequence in the next iteration.
            if (buffer[0] == EOFSequence[index])
                index++;
            // if the current byte doesn't follow the EOF sequence set
            // the index back to the first byte of the EOF sequence.
            else
                index = 0;

            // if the index has reached the end of the EOF sequence, then
            // exit the loop to store the text.
            if (index >= EOFSequenceLen)
                break;
        }

        // check if the EOF sequence has indeed been read.
        if (index >= 8)
        {
            // write the inputted text into the output image after
            // the end of file bytes.
            fwrite(hiddenText, textlen, 1, outimage);

            // message has been stored.
            messageStored = 1;
        }
        // end of operations for png file.
    }

    // Close all opened files and free all the data allocated for the
    // hidden text string to prevent memory leaks.
    fclose(inimage);
    fclose(outimage);
    free(hiddenText);

    // print the appropriate message after all operations have finished.
    if (messageStored == 0)
    {
        // exit with error code 5 if message was not stored.
        printf("Could not store message.\n");
        return 5;
    }
    else
    {
        // exit with code 0 (signifying success) if message was stored.
        printf("Message successfully stored.\n");
        return 0;
    }
}


