///////////////////////////////////////////////////////////////////////////////
/// Variable settings for the reorder buffer
///
/// @file rob_config.hpp
///////////////////////////////////////////////////////////////////////////////

#ifndef ROB_CONFIG_H_INCLUDED
#define ROB_CONFIG_H_INCLUDED

// number of entries in the Reorder buffer
#define ROB_SIZE 32 

// number of FP reservation stations
#define NUM_FP_ADDER_RS 	 4
#define NUM_FP_MULTIPLIER_RS 3
#define NUM_LOAD_RS			 4
#define NUM_STORE_RS		 4
#define NUM_INT_RS			 4

// Instruction Queue Size
#define INSTRUCTION_QUEUE_SIZE 64

// Execution Times
#define FP_ADD_SUB_CYCLE_TIME 3
#define FP_MULTIPLY_CYCLE_TIME 6
#define FP_DIVIDE_CYCLE_TIME 13
#define LOAD_CYCLE_TIME 2
#define STORE_CYCLE_TIME 1
#define INTEGER_INSTRUCTION_CYCLE_TIME 1

#endif // ROB_CONFIG_H_INCLUDED