/* 
 * File:   main.cpp
 * Author: stkerr
 *
 * Created on January 31, 2012, 11:20 PM
 */

#include <cstdlib>
#include <stdio.h>
#include <list>

#include "text_symbols.h"
#include "plt_symbols.h"
#include "minimize.h"
#include "randomize.h"

using namespace std;

/**
 * Reads in the libc file to a buffer
 * @param path The filepath to the libc file
 * @param length The function will write the length of the read in libc file
 * @return The buffer containing the libc
 */
unsigned char * read_libc(char* path, unsigned long* length)
{
    FILE* fd;

    /* Open the file */
    fd = fopen(path, "r");

    if (fd == NULL)
    {
        printf("Could not open libc file: %s\n", path);
        return 0x0;
    }

    /* Seek to the end and record the length */
    fseek(fd, 0L, SEEK_END);
    *length = ftell(fd);

    /* Return to the beginning */
    fseek(fd, 0L, SEEK_SET);

    /* Allocate a buffer of the appropriate length */
    unsigned char * buffer = (unsigned char *) malloc(sizeof (unsigned char) * *length);

    /* Copy the file into the buffer */
    fread(buffer, *length, 1, fd);

    /* Close the file */
    fclose(fd);

    /* Return the buffer */
    return buffer;
}

/**
 * Writes the modified libc to disk
 * @param path The filepath to write to
 * @param libc The buffer containing the modified libc to write out
 * @param length The length of the libc buffer
 * @return 
 */
int write_libc(char* path, unsigned char *libc, unsigned long* length)
{
    /* Open the file for writing */
    FILE* fd = fopen(path, "w");

    /* Did the file open succeed? */
    if (fd == NULL)
    {
        printf("Could not open libc file for writing: %s\n", path);
        return -1;
    }

    /* Write to disk */
    fwrite(libc, sizeof (unsigned char), *length, fd);

    /* Close the file */
    fclose(fd);

    return 0;
}

/*
 * 
 */
int main(int argc, char** argv)
{
    unsigned long length = -1;
    unsigned char * libc = read_libc((char*) "/Users/stkerr/Documents/Research/Marlin/libc.lib.so", &length);

    list<text_symbol> minimizeList, randomizeList;
    minimizeList.push_back(text_symbols::rand);
    minimizeList.push_back(text_symbols::do_rand);
    minimizeList.push_back(text_symbols::rand_r);
    minimizeList.push_back(text_symbols::execl);
    minimizeList.push_back(text_symbols::execle);
    minimizeList.push_back(text_symbols::execlp);
    minimizeList.push_back(text_symbols::execv);
    minimizeList.push_back(text_symbols::execve);

    list<text_symbol>::iterator it;
    for (it = minimizeList.begin(); it != minimizeList.end(); it++)
        minimize_function(libc, *it);

    list<deadzone> deadzones;
    for (it = minimizeList.begin(); it != minimizeList.end(); it++)
    {
        deadzone temp;
        temp.address = (*it).address;
        temp.length = (*it).length;
        deadzones.push_back(temp);
    }
    
    randomizeList.push_back(text_symbols::abs);
    randomizeList.push_back(text_symbols::recv);
    randomizeList.push_back(text_symbols::send);

    list<text_record*> randomizeResults;
    randomize_functions(libc, (char*)"/Users/stkerr/Documents/Research/Marlin/randomization.map", randomizeList, deadzones, &randomizeResults);

    int retVal = write_libc((char*) "/Users/stkerr/Documents/Research/Marlin/libc.lib.so.MODIFIED", libc, &length);

    return retVal;
}

