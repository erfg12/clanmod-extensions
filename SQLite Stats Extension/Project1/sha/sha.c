/*
 *  sha.cpp
 *
 *  Copyright (C) 1998, 2009
 *  Paul E. Jones <paulej@packetizer.com>
 *  All Rights Reserved
 *
 *****************************************************************************
 *  $Id: sha.c 12 2009-06-22 19:34:25Z paulej $
 *****************************************************************************
 *
 *  Description:
 *      This utility will display the message digest (fingerprint) for
 *      the specified file(s).
 *
 *  Portability Issues:
 *      None.
 */

#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#endif
#include <fcntl.h>
#include "sha1.h"

/*
 *  Function prototype
 */
void usage();


/*  
 *  main
 *
 *  Description:
 *      This is the entry point for the program
 *
 *  Parameters:
 *      argc: [in]
 *          This is the count of arguments in the argv array
 *      argv: [in]
 *          This is an array of filenames for which to compute message
 *          digests
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *
 */
/*int main(int argc, char *argv[])
{
    SHA1Context sha;             
    FILE        *fp;              
    char        c;           
    int         i;                
    int         reading_stdin;      
    int         read_stdin = 0;   

    if (argc > 1 && (!strcmp(argv[1],"-?") ||
        !strcmp(argv[1],"--help")))
    {
        usage();
        return 1;
    }

    for(i = 0; i < argc; i++)
    {
        if (i == 0)
        {
            i++;
        }

        if (argc == 1 || !strcmp(argv[i],"-"))
        {
#ifdef WIN32
            setmode(fileno(stdin), _O_BINARY);
#endif
            fp = stdin;
            reading_stdin = 1;
        }
        else
        {
            if (!(fp = fopen(argv[i],"rb")))
            {
                fprintf(stderr,
                        "sha: unable to open file %s\n",
                        argv[i]);
                return 2;
            }
            reading_stdin = 0;
        }

        if (reading_stdin)
        {
            if (read_stdin)
            {
                continue;
            }

            read_stdin = 1;
        }

        SHA1Reset(&sha);

        c = fgetc(fp);
        while(!feof(fp))
        {
            SHA1Input(&sha, &c, 1);
            c = fgetc(fp);
        }

        if (!reading_stdin)
        {
            fclose(fp);
        }

        if (!SHA1Result(&sha))
        {
            fprintf(stderr,
                    "sha: could not compute message digest for %s\n",
                    reading_stdin?"STDIN":argv[i]);
        }
        else
        {
            printf( "%08X %08X %08X %08X %08X - %s\n",
                    sha.Message_Digest[0],
                    sha.Message_Digest[1],
                    sha.Message_Digest[2],
                    sha.Message_Digest[3],
                    sha.Message_Digest[4],
                    reading_stdin?"STDIN":argv[i]);
        }
    }

    return 0;
}*/

/*  
 *  usage
 *
 *  Description:
 *      This function will display program usage information to the
 *      user.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *
 */
void usage()
{
    printf("usage: sha <file> [<file> ...]\n");
    printf("\tThis program will display the message digest\n");
    printf("\tfor files using the Secure Hashing Algorithm (SHA-1).\n");
}
