/* 
 * File:   randomize.h
 * Author: stkerr
 *
 * Created on February 2, 2012, 1:57 PM
 */

#ifndef RANDOMIZE_H
#define	RANDOMIZE_H

#include "text_symbols.h"
#include "jumppatching.h"

#include <list>
#include <map>

/*
 * Represents an area of dead code that randomization can use for shuffling
 */
typedef struct deadzone_struct
{
    unsigned long address;
    unsigned long length;
} deadzone ;

bool deadzone_compare(deadzone first, deadzone second);

int randomize_functions(unsigned char *buffer, char* randomMapPath, std::list<text_symbol> symbols, std::list<deadzone> deadzones, std::list<text_record*> *results);

std::map<char*, std::list<jumppatching*>* >* prepare_patch_database(std::map<char*, int> relocations);
        
#endif	/* RANDOMIZE_H */

