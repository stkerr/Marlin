#include "text_symbols.h"

/**
 * Compares two text symbols by size 
 * @param first The first symbol
 * @param second The second symbol
 * @return True is first is smaller than second, otherwise, false
 */
bool text_symbol_compare(text_symbol first, text_symbol second)
{
    if(first.length <= second.length)
        return true;
    return false;
}
