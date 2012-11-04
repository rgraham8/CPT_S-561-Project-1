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

// The states of an ROB entry
enum STATE
{
	EMPTY,
	ISSUE,
	EXECUTING,
	EXECUTION_COMPLETE,
	WRITING,
	COMMIT
};

class Register
{
	public:
		bool m_busy;
		
		Register(bool busy): m_busy(busy){}
	
};
	
///////////////////////////////////////////////////////////////////////////////
// Floating Point Register
///////////////////////////////////////////////////////////////////////////////
class FP_Register : public Register
{
	public:
		float data;
		
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
		bool busy;
		int data;
		
		Int_Register(void):
			busy(false),
			data(0){}
};

///////////////////////////////////////////////////////////////////////////////
// Instruction class representing instruction to be executed
///////////////////////////////////////////////////////////////////////////////	
class Instruction
{
	public:
		Instruction_Set m_instruction;
		Register* m_destination_register;
		Register* m_source_register_one;
		Register* m_source_register_two;
		unsigned int m_execution_counter;
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
		unsigned int m_entry_number; // entry number
		ROB_Entry* m_next; ///< next pointer for circular buffer
		
		///////////////////////////////////////////////////////////////////////
		// Constructor
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
		static unsigned int s_next_id; // used to increment each entry number
};

///////////////////////////////////////////////////////////////////////////////
/// The Reorder Buffer
/// Contains a defined number of entries
///////////////////////////////////////////////////////////////////////////////
class ROB
{
	public:
		///////////////////////////////////////////////////////////////////////
		/// Constructor: Initializes the ROB circular buffer
		///////////////////////////////////////////////////////////////////////
		ROB(void);
		
		///////////////////////////////////////////////////////////////////////
		/// Destructor: Deallocate memory allocated in constructor
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
		ROB_Entry* m_rob_head; // head of ROB buffer
		
		// Commit stage increments this to remove instructions
		// from the list (FULL when head == tail)
		ROB_Entry* m_head;
		
		// Dispatch stage increments this to add instructions
		// to the list
		ROB_Entry* m_tail;
};

#endif // ROB_BUFFER_H_INCLUDED

