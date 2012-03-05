#ifndef LIBC_JUMP_PATCH
#define LIBC_JUMP_PATCH

#include <list>
class jumppatching
{
public:
	jumppatching(arguments);
	~jumppatching();

	/*
		This is the offset in the function to a jump to __i686.get_pc_thunk.bx
		Since __i686.get_pc_thunk.bx is not relocated, it is possible to determine
		the new location of the function, recalculate the offset and then record that
		new offset 

		Thunk_bx: 0001ec9b
		Thunk_cx: 0001fc6c
	*/
	unsigned int thunk_bx_offset;
	unsigned int thunk_cx_offset;

	/*
		This is a list of <int, string> pairs. Each pair represents a 'call' instruction.
		The int is the offset into the function that the call occurs and the string is
		the name of the function that is being called. Since call instructions are a 4 byte
		offset, it will be possible to calculate the address these functions need to be
		remapped to
	*/
	std::list< std::pair<unsigned int, std::string> > function_call_patches;
};

#endif