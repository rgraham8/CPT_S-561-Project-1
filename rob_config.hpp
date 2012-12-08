///////////////////////////////////////////////////////////////////////////////
/// Variable settings for the reorder buffer
///
/// @file rob_config.hpp
///////////////////////////////////////////////////////////////////////////////

#ifndef ROB_CONFIG_H_INCLUDED
#define ROB_CONFIG_H_INCLUDED

#define ROB_SIZE 32 ///< number of entries in the Reorder buffer

#define MEMORY_SIZE_BYTES 1000 ///< size of memory in bytes

//#define MAX_ISSUE_PER_CYCLE 1 ///< max number of instructions that can
							 ///< be issued per cycle
							
#define NUM_FP_ADDER_RS 	 3 ///< number of FP reservation stations
#define NUM_FP_MULTIPLIER_RS 3 ///< number of FP multiply reservation stations
#define NUM_LOAD_RS			 3 ///< number of load reservation stations
#define NUM_STORE_RS		 4 ///< number of store reservation stations
#define NUM_INT_RS			 4 ///< number of integer unit reservation stations

#define INSTRUCTION_QUEUE_SIZE 64 ///< Instruction Queue Size

#define NUM_FP_REGISTERS 16 ///< number of FP registers
#define NUM_INT_REGISTERS 16 ///< number of interget registers

// Execution Times
#define FP_ADD_SUB_CYCLE_TIME 3 ///< add/sub exectuion time
#define FP_MULTIPLY_CYCLE_TIME 6 ///< fp multiply execution time
#define FP_DIVIDE_CYCLE_TIME 13 ///< fp divide execution time
#define LOAD_CYCLE_TIME 1 ///< load execution time
#define STORE_CYCLE_TIME 1 ///< store execution time
#define INTEGER_INSTRUCTION_CYCLE_TIME 1 ///< integer instruction excution time

#endif // ROB_CONFIG_H_INCLUDED