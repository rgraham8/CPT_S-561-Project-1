///////////////////////////////////////////////////////////////////////////////
/// This file implements Tamasulo's Algorithm with Reorder Buffer
///
/// @file rob_buffer.hpp
///////////////////////////////////////////////////////////////////////////////

#ifndef ROB_BUFFER_H_INCLUDED
#define ROB_BUFFER_H_INCLUDED

#include <stdint.h>
#include <iostream>
#include <assert.h>

#include "rob_config.hpp"

enum Instruction_Set
{
	NOOP,
	ADD,
	MULTIPLY,
	DIVIDE,
	SUBTRACT
};

/// The states of an ROB entry
enum STATE
{
	EMPTY,
	ISSUE,
	EXECUTING,
	EXECUTION_COMPLETE,
	WRITING,
	COMMIT
};

///////////////////////////////////////////////////////////////////////////////
/// Registe class representing a single floating point or interger register
///////////////////////////////////////////////////////////////////////////////
class Register
{
	public:
		bool m_busy; ///< state of this register's data
		
		///////////////////////////////////////////////////////////////////////
		/// Constructor
		///
		/// @param[in] busy state of this registers data
		///////////////////////////////////////////////////////////////////////
		Register(bool busy): m_busy(busy){}
	
};
	
///////////////////////////////////////////////////////////////////////////////
// Floating Point Register
///////////////////////////////////////////////////////////////////////////////
class FP_Register : public Register
{
	public:
		float data; ///< register's data
		
		///////////////////////////////////////////////////////////////////////
		FP_Register(void):
			Register(false),
			data(0){}
};

///////////////////////////////////////////////////////////////////////////////
// Interger register 
///////////////////////////////////////////////////////////////////////////////
class Int_Register : public Register
{
	public:
		int data; ///< register's data
		
		///////////////////////////////////////////////////////////////////////
		Int_Register(void):
			Register(false),
			data(0){}
};

///////////////////////////////////////////////////////////////////////////////
// Instruction class representing instruction to be executed
///////////////////////////////////////////////////////////////////////////////	
class Instruction
{
	public:
		Instruction_Set m_instruction; ///< actually instruction to be executed
		Register* m_destination_register; ///< destination register of result
		Register* m_source_register_one; ///< source 1 register
		Register* m_source_register_two; ///< source 2 register
		unsigned int m_execution_counter; ///< remaining execution time counter
		
		///////////////////////////////////////////////////////////////////////
		Instruction(void):
			m_instruction(NOOP),
			m_destination_register(NULL),
			m_source_register_one(NULL),
			m_source_register_two(NULL),
			m_execution_counter(0){}
};

///////////////////////////////////////////////////////////////////////////////
// One entry in the ROB 
///////////////////////////////////////////////////////////////////////////////
class ROB_Entry
{
	public:
		bool m_busy; ///< whether unit is busy
		Instruction m_instruction; ///< instruction being issued/executed
		STATE m_state; ///< the state of the unit
		long* m_destination; ///< the destination register
		unsigned int m_entry_number; ///< entry number
		ROB_Entry* m_next; ///< next pointer for circular buffer
		
		///////////////////////////////////////////////////////////////////////
		ROB_Entry(void):	
			m_busy(false),
			m_instruction(),
			m_state(EMPTY),
			m_destination(NULL),
			m_next(NULL)
   		{
			m_entry_number = ROB_Entry::s_next_id++;
		}
	
	private:
		static unsigned int s_next_id; ///< unique entry number
};

///////////////////////////////////////////////////////////////////////////////
/// The Reorder Buffer
/// Contains a defined number of entries
///////////////////////////////////////////////////////////////////////////////
class ROB
{
	public:
		///////////////////////////////////////////////////////////////////////
		ROB(void);
		
		///////////////////////////////////////////////////////////////////////
		~ROB(void);
		
		///////////////////////////////////////////////////////////////////////
		/// Issue an instruction the to reorder buffer
		///
		/// @param[in] issued_instruction the instruction to add to the ROB
		///
		/// @returns FALSE if a slot was not available in the ROB. TRUE otherwise
		///////////////////////////////////////////////////////////////////////
		bool issue_instruction(Instruction issued_instruction);
		
		///////////////////////////////////////////////////////////////////////
		/// Process all instructions in the reorder buffer
		///////////////////////////////////////////////////////////////////////
		void process_instructions(void);
		
		///////////////////////////////////////////////////////////////////////
		/// Process all instructions in the reorder buffer
		///
		/// @param[in] rob_entry the rob entry containing instruction to commit
		///
		/// @returns TRUE if instruction was committed
		///////////////////////////////////////////////////////////////////////
		bool commit_instruction(ROB_Entry*& rob_entry);
		
	private:
		ROB_Entry* m_rob_head; ///< Head of ROB buffer
		
		ROB_Entry* m_head; ///< Commit stage increments this to remove instructions
						   ///< from the list (FULL when head == tail)
						   ///<
		
		ROB_Entry* m_tail; ///< Dispatch stage increments this to add instructions
						   ///< to the list
						   ///<
		
		int32_t m_memory[MEMORY_SIZE_BYTES]; ///< Pointer to hardware memory
		
		FP_Register m_fp_register[NUM_FP_REGISTERS]; ///< array of FP registers
		Int_Register m_int_register[NUM_INT_REGISTERS]; ///< array of integer registers
};

#endif // ROB_BUFFER_H_INCLUDED

