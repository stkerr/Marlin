#include "randomize.h"
#include "text_symbols.h"

#include <set>
#include <list>
#include <iostream>
#include <fstream>

using namespace std;

set<int> deadzone_histogram(list<deadzone> *zones)
{
    set<int> results;
    
    list<deadzone>::iterator it;
    for(it = zones->begin(); it != zones->end(); it++)
    {
        results.insert(it->length);
    }
    
    set<int>::iterator r_it;
    for(r_it = results.begin(); r_it != results.end(); r_it++)
    {
        cout << hex << "Empty zone of length 0x" << *r_it << endl;
    }
    
    return results;
}

/**
 * Returns the comparison by size of two dead zones
 * @param first The first deadzone
 * @param second The second deadzone
 * @return True if first is smaller than than second, otherwise false
 */
bool deadzone_compare(deadzone first, deadzone second)
{
    if (first.length <= second.length)
        return true;
    return false;
}

bool patches_needed(unsigned char *buffer, text_symbol symbol)
{
    return false;
}

/**
 * Performs randomization of the given functions in the buffer and writes out a randomization map
 * @param buffer The buffer containing the functions
 * @param randomMapPath Records the results of randomization here. This is needed for patching PLT entries at runtime.
 * @param symbols The list of symbols that should be shuffled around
 * @param deadzones A list of deadzones that can be used for shuffling and moving functions around
 * @param results The function will record the new values of the functions after randomization here
 * @return 0 on success, non-zero on failure
 */
int randomize_functions(unsigned char *buffer, char* randomMapPath, list<text_symbol> symbols, list<deadzone> deadzones, list<text_record*> *results)
{
    cout << "Stating randomization phase" << endl;
    cout << "Deadzone Count: " << deadzones.size() << endl;
    deadzone_histogram(&deadzones);
   
    
    /* Sort the functions by size first (smallest to largest, then reverse it) */
    symbols.sort(text_symbol_compare);
    symbols.reverse();

    /* Sort the deadzones by size, smallest to largest */
    deadzones.sort(deadzone_compare);

    /* Iterate through each function */
    list<text_symbol>::iterator it;

    /* Seed the random */
    srand(time(NULL));

    for (it = symbols.begin(); it != symbols.end(); it++)
    {
        cout << hex << "Processing " <<  it->symbolName << "() from 0x" << it->address << " length: 0x" << it->length << endl;
        
        /* Check for if we need to patch jump instructions */
        if (patches_needed(buffer, *it) == true)
        {
            cout << "Jump patching needed. Not implemented yet." << endl;
        }

        /* Iterate through the deadzones and see if we can find a place to put this function */
        list<deadzone>::iterator d_it;
        deadzone space = {0, 0};
        for (d_it = deadzones.begin(); d_it != deadzones.end(); d_it++)
        {
            /* Don't randomize into the old location */
            
            
            if (d_it->length >= it->length)
            {
                cout << hex << "Randomizing " << it->symbolName << " into 0x" << d_it->address << " length: 0x" << d_it->length << endl;

                /* Is there a little wiggle room for the function? */
                int difference = d_it->length - it->length;

                cout << "Extra space: 0x" << difference << endl;
                /* There's some space, so lets randomize the offset a bit */
                if (difference > 0)
                {
                    /* Make a random shift */
                    int shift = rand() % difference;
                    
                    cout << "Randomly shifting " << it->symbolName << "() 0x" << shift << " bytes" << endl;
                    
                    /* Record the space we're putting it */
                    space.address = d_it->address+shift;
                    space.length = d_it->length;
                }
                else
                {
                    /* Record the space we're putting it */
                    space.address = d_it->address;
                    space.length = d_it->length;
                }


                /* Erase this deadzone */
                d_it = deadzones.erase(d_it);

                /* Break the loop */
                break;
            }
        }
        if (space.address == 0 && space.length == 0)
        {
            cout << "Could not find a space to randomize function " << it->symbolName << "() into." << endl;
            continue;
        }

        /* Perform the memory copy now */
        for (int i = 0; i < space.length; i++)
        {
            *(buffer + i + space.address) = buffer[it->address + i];
        }

        /* We just moved a function, put its old space into the deadzones */
        deadzone newdead;
        newdead.address = it->address;
        newdead.length = it->length;
        deadzones.push_back(newdead);

        /* RET out the area the function used to live */
        for (int i = 0; i < it->length; i++)
        {
            *(buffer + it->address + i) = 0xC3; // RET
        }

        /* Resort the deadzones */
        deadzones.sort(deadzone_compare);

        /* Log the results of this randomization */
        text_record* rec = new text_record;
        rec->symbol.address = it->address;
        rec->symbol.length = it->length;
        rec->symbol.address = space.address;
        strcpy(rec->symbol.symbolName, it->symbolName);
        rec->old_address = it->address;
        
        /* TODO: Add in any left over space back to the deadzones */

        /* Record the results */
        results->push_back(rec);
    }

    /* Write the results to disk */

    FILE* file = fopen(randomMapPath, "w");

    list<text_record*>::iterator results_it;
    for (results_it = results->begin(); results_it != results->end(); results_it++)
    {
        cout << hex << "Address: 0x" << (*results_it)->symbol.address << " Length: 0x" << (*results_it)->symbol.length << endl;
        cout << "Next results_it entry " << (*results_it)->symbol.symbolName << endl;

        fprintf(file, "%8x:%8x:%8x:%s\n", (*results_it)->old_address, (*results_it)->symbol.address, (*results_it)->symbol.length, (*results_it)->symbol.symbolName);
        /*
        file << ":";
        file << (*results_it)->old_address; // old address
        file << ":";
        file << hex << (*results_it)->symbol.address; // new address
        file << ":";
        file << hex << (*results_it)->symbol.length; // length
        file << "\n";
        file << (*results_it)->symbol.symbolName; // symbol name
            */
    }

    fclose(file);


    return 0;
}
