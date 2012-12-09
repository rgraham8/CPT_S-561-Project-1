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
		
		std::cout<<"Enter number of instructions issued per cyle: "<<std::endl;
		
		int num_issue_count = 1;
		std::cin>>num_issue_count;
		reorder_buffer.set_num_issue_per_cyle(num_issue_count);

		std::cout<<"\nSTATE\tINSTRUCTION\tEXECUTION COUNTER\tRESULT\n"<<std::endl;
		
		while (reorder_buffer.process_instructions()){}
		
		std::cout<<"EOP"<<std::endl;
		
		std::cout<<"-- Statistics --"<<std::endl;
		std::cout<<"Average Issues Per Cycle: "<<reorder_buffer.get_avg_num_of_issues()<<std::endl;
		std::cout<<"Average Writes Per Cycle: "<<reorder_buffer.get_avg_num_of_writes()<<std::endl;
		std::cout<<"Average Commits Per Cycle: "<<reorder_buffer.get_avg_num_of_commits()<<std::endl;
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

