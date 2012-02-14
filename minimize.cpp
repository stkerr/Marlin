/**
 * Fills out the given function in the buffer with RET instructions
 * 
 * TODO: Implement a dummy code to return 0xDEADCODE
 * @param buffer The buffer containing the code
 * @param address The address of the function to minimize
 * @param length The length of the function
 * @return 0 is success, non-zero is failure
 */

#include "minimize.h"

int minimize_function(unsigned char * buffer, text_symbol symbol)
{
    for(long i = 0; i < symbol.length; i++)
    {
        *(buffer+symbol.address+i) = 0xC3; // RET opcode
    }
    
    return 0;
}
