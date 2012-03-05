#include "jumppatching.h"
#include <iostream>
using namespace std;

//const unsigned int jumppatching::thunk_bx_offset = 0x0001ec9b; /* Hardcoded for libc */
//const unsigned int jumppatching::thunk_cx_offset = 0x0001fc6c; /* Hardcoded for libc */

map<char*, list<jumppatching*> > patch_database; // <symbol name, list of patches>

jumppatching::jumppatching()
{
	
}

/*
	Returns true if the patch succeeded, false if it failed
*/
bool perform_patch(unsigned char *buffer, text_symbol symbol, int new_location)
{
    
}

/* Calculates and updates the jump distance for teh given offset and destination address */
bool lowlevel_patch(unsigned char *buffer, int offset, int dest)
{
	cout << hex << "Patching: 0x" << offset << " to 0x" << dest << endl;

	int distance_to_jump = dest-offset-4; // 4 bytes for the instruction

	*((int*)(buffer+offset)) = distance_to_jump;

	cout << hex << "Patched jump: 0x" << distance_to_jump << endl;

	return true;
}