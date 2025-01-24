// go through writemessage.c first.
// ---------------------------------------------------------------------------------------------
// this program reads hidden messages embedded through steganography for the following types:
// BMP, JPG, PNG.
//
// only works if:
// BMP steganography is done using LSB method from the start of the pixel array.
// JPG steganography is done by storing the data after End Of File.
// PNG steganography is done by storing the data after End Of File.
// ---------------------------------------------------------------------------------------------

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "helpers.h"

// argc is the number of command line arguments given.
// argv is an array of strings containing all the arguments.
int main(int argc, char* argv[])
{
    // if the number of arguments is not 2, i.e, ONLY the image path
    // of the image with a secret message is not given, exit with
    // an error code -1.
    if (argc != 2 && argc != 3)
    {
        printf("Incorrect usage.\nCorrect usage: ./readmessage <steganographyimage> (optional)<passkey>\n");
        return -1;
    }

    // store the image path and passkey in its own separate string.
    char* imagepath = argv[1];
    char* passkey = NULL;
    if (argc == 3)
        passkey = argv[2];

    // open the image file from the imagepath
    // if the image path is not valid, exit with error code 1.
    FILE* image = fopen(imagepath, "r");
    if (image == NULL)
    {
        printf("Could not open image: %s\n", imagepath);
        return 1;
    }

    // a variable to keep track of if the secret message has
    // been printed or not.
    // 0 -> false (message not printed).
    // 1 -> true (message printed).
    int textPrinted = 0;

    // check the file type and store it in a variable.
    // if file type is unsupported, exit with error code 2.
    int fileType = checkFileType(image, NULL);
    if (fileType == UNSUPPORTEDTYPE)
    {
        printf("Unsupported file type.\nSupported file types are BMP, JPG, PNG.\n");
        return 2;
    }

    // execute operations according to file type.

    // operations to be done if image type is BMP:
    if (fileType == BMP)
    {
        // create a buffer with space for 8 BYTEs of data
        BYTE buffer[BYTESIZE];

        // readHeaderForBMP() reads the header and returns
        // the value of where the pixel array, i.e, all the RGB
        // values of the image starts.
        int pixelArrayOffset = readHeaderForBMP(image);

        // move the "cursor" to where the pixel array starts.
        fseek(image, pixelArrayOffset, SEEK_SET);

        // read the data from the pixel array LSBs and print it.
        while (fread(buffer, BYTESIZE, 1, image) != 0)
        {
            // if the hidden text was completely printed, exit the loop.
            if (textPrinted == 1)
                break;
            // the readCharFromLSBAndPrint() function, as the name
            // says, reads the LSBs of the bytes, forms the character
            // that was stored, and prints it. It returns 1 (true)
            // if all the text was printed. (i.e, the special byte [0000 0000]
            // that was used to signify the end of text was reached).
            textPrinted = readCharFromLSBAndPrint(buffer, passkey);
        }
        // end of operations for bmp file.
    }
    // operations to be done if image type is JPG:
    else if (fileType == JPG)
    {
        // the two bytes (or 8 bits) that signify the end of file in
        // a jpg file combined into a single 16 bit number.
        int endOfImage = 0xFFD9;

        // variable to store the just read bytes.
        int currBytes = 0x0000;

        // a buffer of the size of the number of signature bytes at
        // the beginning of a file (2).
        BYTE buffer[SIGNATUREBYTESIZE];

        // while the just read bytes is not equal the the EOF bytes,
        // read the file and store the bytes in the buffer.
        while (currBytes != endOfImage)
        {
            fread(buffer, SIGNATUREBYTESIZE, 1, image);

            // this is just syntax to combine two 8-bit numbers into a
            // single 16-bit number. Look up the operators used here
            // if interested.
            currBytes = buffer[0] << 8 | buffer[1];
        }

        // once the end of file bytes have been reached, all of the data
        // stored comes after it. So read all of the data and print it.
        while (fread(buffer, SIGNATUREBYTESIZE, 1, image) != 0)
        {
            if (passkey != NULL)
            {
                buffer[0] = decryptChar(buffer[0], passkey);
                buffer[1] = decryptChar(buffer[1], passkey);
            }
            printf("%c%c", buffer[0], buffer[1]);
        }

        // the hidden text has been printed.
        textPrinted = 1;
        // end of operations for jpg file.
    }
    // operations to be done if image type is PNG:
    else if (fileType == PNG)
    {
        // buffer to store ONE BYTE of data.
        BYTE buffer[1];

        // the sequence of bytes that signify the end of file in a png.
        int EOFSequence[] = {0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82};

        // the number of bytes in the EOF sequence.
        int EOFSequenceLen = 8;

        // index to keep track of the byte in the EOF sequence that has been
        // found in the file.
        int index = 0;

        // keep reading the input file until the EOF sequence is reached.
        while (fread(buffer, 1, 1, image) != 0)
        {
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
            // read and print whatever was stored after the EOF.
            while (fread(buffer, 1, 1, image) != 0)
            {
                if (passkey != NULL)
                    buffer[0] = decryptChar(buffer[0], passkey);

                printf("%c", buffer[0]);
            }
            // the text has been printed.
            textPrinted = 1;
        }
        // end of operations for png file.
    }

    // close the opened image to prevent memory leak.
    fclose(image);

    // print the appropriate message after all operations have finished.
    if (textPrinted == 1)
    {
        // exit program with code 0 to signify success if the message
        // was read properly.
        printf("Message read successfully.\n");
        return 0;
    }
    else
    {
        // exit the program with code 3 if the message wasnt read properly.
        printf("Could not read message.\n");
        return 3;
    }
}
