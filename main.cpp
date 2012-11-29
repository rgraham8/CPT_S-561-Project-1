///////////////////////////////////////////////////////////////////////////////
/// This file implements Tamasulo's Algorithm with Reorder Buffer
/// 
/// @file main.cpp
///////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <iostream>
#include <assert.h>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

#include "rob_buffer.hpp"
#include "rob_config.hpp"

static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


static std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

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
		int count = 0;
		for (int cycle = 0; cycle < 5; cycle++)
		{
			std::cout<<std::endl<<"CYCLE "<<cycle<<std::endl<<std::endl;
			reorder_buffer.execute_intructions();
			reorder_buffer.issue_instruction();
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

