#include "randomize.h"
#include "text_symbols.h"
#include "jumppatching.h"

#include <map>
#include <set>
#include <list>
#include <iostream>
#include <fstream>


using namespace std;

set<int> deadzone_histogram(list<deadzone> *zones)
{
    set<int> results;

    list<deadzone>::iterator it;
    for (it = zones->begin(); it != zones->end(); it++)
    {
        results.insert(it->length);
    }

    set<int>::iterator r_it;
    for (r_it = results.begin(); r_it != results.end(); r_it++)
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

/*
 * Iterates through every symbol and prepares the list of patches
 * needed for each symbol
 */
map<string, list<jumppatching*>* >* prepare_patch_database(map<string, int> relocations)
{
    map<string, list<jumppatching*>* >* patch_db = new map<string, list<jumppatching*>* >;
    
    patch thunkcx_patch;
    thunkcx_patch.library_offset = relocations["rand"];
    
    strcpy(thunkcx_patch.function_name, "rand");
    thunkcx_patch.offset = 1; // 1 byte in
    thunkcx_patch.patch_length = 4; // 4 byte jump patch
    
    strcpy(thunkcx_patch.function_name, "thunkcx");
    thunkcx_patch.dest_function = jumppatching::thunk_cx_offset;
    thunkcx_patch.dest_offset = 0;
    thunkcx_patch.in_plt = false;
    
    jumppatching* rand_patch = new jumppatching();
    rand_patch->function_call_patches.push_front(thunkcx_patch);
//    rand_patch.reelative_jump_patches.push_front(do_rand_patch);
    
            
    list<jumppatching*> *patches = new list<jumppatching*>;
    patches->push_back(rand_patch);
    
    patch_db->insert(pair<string, list<jumppatching*>* >("rand", patches));
    return patch_db;
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

    /* Record where we are putting these different symbols */
    map<string, int> relocations; // <symbol name, new address>

    for (it = symbols.begin(); it != symbols.end(); it++)
    {
        cout << hex << "Processing " << it->symbolName << "() from 0x" << it->address << " length: 0x" << it->length << endl;

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
                //if (difference > 0)
                if(0 == 1) // do not execute for now
                {
                    /* Make a random shift */
                    int shift = rand() % difference;

                    cout << "Randomly shifting " << it->symbolName << "() 0x" << shift << " bytes" << endl;

                    /* Record the space we're putting it */
                    space.address = d_it->address + shift;
                    space.length = d_it->length;
                }
                else
                {
                    /* Record the space we're putting it */
                    space.address = d_it->address;
                    space.length = d_it->length;
                }

                /* Record the relocation for jump patching later */
                //int erased = current_addresses.erase((char*)(it->symbolName));
                //current_addresses.insert(pair<char*, int>((char*)(it->symbolName), space.address));
                //cout << "Erased: " << erased << endl;

                current_addresses[(it->symbolName)] =  space.address;

                relocations.insert(pair<char*, int>(it->symbolName, space.address));
                cout << "Inserting into relocations: " << it->symbolName << endl;

                cout << hex << current_addresses["rand"] << endl;    
                
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

    cout << endl;
    cout << "Beginning jump patching" << endl;
    cout << endl;

    //bool lowlevel_patch(unsigned char *buffer, int offset, int dest)
    //lowlevel_patch(buffer, )

    /* Perform the jump patching phase */
    map<string, list<jumppatching*>* >* patch_database = prepare_patch_database(relocations);
    cout << "Database size: " << patch_database->size() << endl;
    for (it = symbols.begin(); it != symbols.end(); it++)
    {
        cout << hex << "Processing " << it->symbolName << "() from 0x" << it->address << " length: 0x" << it->length << endl;

        /* Check for if we need to patch jump instructions */
        if (it->jumppatching > NOTDETERMINED)
        {

            if(patch_database->count((*it).symbolName) == 0)
            {
                cout << "Could not find symbol: " << (*it).symbolName << " in patch database" << endl;
                continue;
            }

            if(strcmp((*it).symbolName, "rand") != 0)
                continue;

            int new_location = relocations.at(it->symbolName);

            bool patch_results;

            // hardcoded for rand()
            patch_results = lowlevel_patch(buffer, new_location + 1, jumppatching::thunk_cx_offset);    
            patch_results = lowlevel_patch(buffer, new_location + 22, current_addresses["do_rand"]);    
            continue;

            //map<char*, list<jumppatching*> >::iterator temp_it = patch_database.find((*it).symbolName);
            //if(temp_it == NULL)
                //continue;
            list<jumppatching*>* patches = patch_database->find((*it).symbolName)->second;

            list<jumppatching*>::iterator patch_it;
            for(patch_it = patches->begin(); patch_it != patches->end(); patch_it++)
            {
                /* Grab the patches that refer to jumps to a function */
                //patch function_patch = (*patch_it)->function_call_patches.front();

                
            }            
            

            //patch_results = perform_patch(buffer, *it, new_location);
            if (patch_results == false)
            {
                cout << "Jump patching failed for randomized symbol :" << it->symbolName << endl;
                continue;
            }
            else
            {
                cout << "Jump patching succeeded for symbol: " << it->symbolName << endl;
            }
        }
        else if (it->jumppatching == NOTDETERMINED)
        {
            cout << "Could not determine if jump patching needed. Might experience errors in symbol:" << it->symbolName << endl;
            continue;
        }
        else
        {
            cout << "Jump patching not needed for symbol: " << it->symbolName << endl;
        }
    }

    /* Write the results to disk */

    cout << endl;
    cout << "Recording changes to disk file" << endl;
    cout << endl;

    FILE* file = fopen(randomMapPath, "w");

    list<text_record*>::iterator results_it;
    for (results_it = results->begin(); results_it != results->end(); results_it++)
    {
        cout << hex << (*results_it)->symbol.symbolName << ": Address: 0x" << (*results_it)->symbol.address << " Length: 0x" << (*results_it)->symbol.length << endl;

        fprintf(file, "%8x:%8x:%8x:%s\n", (*results_it)->old_address, (*results_it)->symbol.address, (*results_it)->symbol.length, (*results_it)->symbol.symbolName);
    }

    fclose(file);


    return 0;
}
