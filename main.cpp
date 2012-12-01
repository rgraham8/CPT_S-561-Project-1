///////////////////////////////////////////////////////////////////////////////
/// This file implements Tamasulo's Algorithm with Reorder Buffer
/// 
/// @file main.cpp
///////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <iostream>
#include <assert.h>

#include "rob_buffer.hpp"
#include "rob_config.hpp"

///////////////////////////////////////////////////////////////////////////////
/// Program entry point
///
/// @param[in] argc number command-line of args
/// @param[in] argv array of command-line arguments
///
/// @retval 0 if no problems occured
/// @retval 1 if an error code
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // Initialize the ROB buffer
	try
	{
		ROB reorder_buffer;

		std::cout<<"\nSTATE\tINSTRUCTION\tEXECUTION COUNTER\tRESULT\n"<<std::endl;
		
		for (int cycle = 0; cycle < 30; cycle++)
		{
			reorder_buffer.process_instructions();
		}
	}
	catch (std::runtime_error &e)
	{
		std::cout<<e.what()<<std::endl;
	}
	catch(...)
	{
		std::cout<<"Unknown error"<<std::endl;
	}

	return 0;
}

