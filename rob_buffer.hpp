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
#include <vector>
#include <queue>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>

#include "rob_config.hpp"

enum Instruction_Set
{
	NOOP,
	ADD,
	MULTIPLY,
	DIVIDE,
	SUBTRACT,
	SUBI,
	ADDI
};

/// The states of an ROB entry
enum STATE
{
	EMPTY,
	ISSUE,
	EXECUTING,
	EXECUTION_COMPLETE,
	WRITING,
	COMMIT,
	WAITING
};

enum INSTRUCTION_TYPE
{
	FLOATING_POINT,
	INTEGER,
	LOAD,
	STORE
};

enum REGISTER_TYPE
{
	FP,
	INT
};

// FWD Declaration
class ROB_Entry;

///////////////////////////////////////////////////////////////////////////////
/// Registe class representing a single floating point or interger register
///////////////////////////////////////////////////////////////////////////////
class Register
{
	public:
		bool m_busy; ///< state of this register's data
		ROB_Entry* m_rob_entry; ///< ROB entry currently holding register value
		float m_data; ///< register's data
		REGISTER_TYPE m_type; ///< type of register
		
		///////////////////////////////////////////////////////////////////////
		/// Constructor
		///
		/// @param[in] busy state of this registers data
		///////////////////////////////////////////////////////////////////////
		Register(REGISTER_TYPE type): m_busy(false), m_rob_entry(NULL), m_type(type)
		{
			srand (time(NULL));
			m_data = rand() % 100;
		}
	
};
	
class Reservation_Station
{
	public:
		Reservation_Station():
			m_execution_counter(0),
			m_instruction(NOOP),
			m_source_one(0),
			m_source_two(0),
			m_rob_entry(NULL),
			m_waiting_rob_entry_value_one(NULL),
			m_waiting_rob_entry_value_two(NULL),
			m_result(0){}
			
		unsigned int m_execution_counter; ///< remaining execution time counter
		Instruction_Set m_instruction;	///< instruction to execute
		float m_source_one; ///< source 1 
		float m_source_two; ///< source 2 
		ROB_Entry* m_rob_entry; ///< ROB Entry to write result to
		ROB_Entry* m_waiting_rob_entry_value_one; ///< ROB Entry containing required register
		ROB_Entry* m_waiting_rob_entry_value_two; ///< ROB Entry containing required register
		double m_result; ///< result of execution
};

///////////////////////////////////////////////////////////////////////////////
// Floating Point Register
///////////////////////////////////////////////////////////////////////////////
class FP_Register : public Register
{
	public:
		///////////////////////////////////////////////////////////////////////
		FP_Register(): Register(FP){}
};

///////////////////////////////////////////////////////////////////////////////
// Interger register 
///////////////////////////////////////////////////////////////////////////////
class Int_Register : public Register
{
	public:
		///////////////////////////////////////////////////////////////////////
		Int_Register(): Register(INT){}
};

///////////////////////////////////////////////////////////////////////////////
// Instruction class representing instruction to be executed
///////////////////////////////////////////////////////////////////////////////	
class Instruction
{
	public:
		Instruction_Set m_instruction; ///< actual instruction to be executed
		Register* m_destination_register; ///< destination register
		Register* m_source_register_one; ///< source 1 register
		Register* m_source_register_two; ///< source 2 register
		int 	  m_load_store_offset; ///< memory offset
		float 	  m_immediate_value; ///< immediate value
		INSTRUCTION_TYPE m_type; ///< type of operation
		std::string m_raw_instruction; ///< raw instruction read from file
		std::string m_loop_name; ///< loop name
		///////////////////////////////////////////////////////////////////////
		Instruction(void):
			m_instruction(NOOP),
			m_destination_register(NULL),
			m_source_register_one(NULL),
			m_source_register_two(NULL),
			m_load_store_offset(0),
			m_immediate_value(0),
			m_type(FLOATING_POINT),
			m_raw_instruction(""),
			m_loop_name("")
			{}
};

///////////////////////////////////////////////////////////////////////////////
// One entry in the ROB 
///////////////////////////////////////////////////////////////////////////////
class ROB_Entry
{
	public:
		std::vector<Instruction>::iterator m_instruction; ///< instruction being issued/executed
		STATE m_state; ///< the state of the unit
		Register* m_destination_register; ///< the destination register
		unsigned int m_entry_number; ///< entry number
		double m_value; ///< value written from reservation station
		ROB_Entry* m_next; ///< next pointer for circular buffer
		Reservation_Station* m_reservation_station;
		bool m_busy; ///< whether this entry is still executing
		
		///////////////////////////////////////////////////////////////////////
		ROB_Entry(void):	
			m_instruction(),
			m_state(EMPTY),
			m_destination_register(NULL),
			m_value(0),
			m_next(NULL),
			m_reservation_station(NULL),
			m_busy(true)
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
		/// Check is a reservation station is available for this instruction
		///
		/// @param[in] instruction The instruction to be executed
		///
		/// @returns TRUE if a reservation station is available
		///////////////////////////////////////////////////////////////////////
		bool reservation_station_available(std::vector<Instruction>::iterator& instruction, ROB_Entry*& rob_entry);
		
		///////////////////////////////////////////////////////////////////////
		/// Issue an instruction the to reorder buffer
		///////////////////////////////////////////////////////////////////////
		void issue_instruction(void);
		
		///////////////////////////////////////////////////////////////////////
		/// Process all instructions in the reorder buffer
		///////////////////////////////////////////////////////////////////////
		void process_instructions(void);
		
		///////////////////////////////////////////////////////////////////////
		/// Process all instructions in the reorder buffer
		///
		/// @param[in] rob_entry the rob entry containing instruction to commit
		///////////////////////////////////////////////////////////////////////
		void commit_instruction(void);
		
		///////////////////////////////////////////////////////////////////////
		/// Execute the instructions in all the reservation stations
		///////////////////////////////////////////////////////////////////////
		void execute_intructions(void);
		
	private:
		
		void process_reservation_station(Reservation_Station* rs, std::vector<Reservation_Station*>& waiting_units, bool from_queue);
		
		ROB_Entry* m_head; ///< Commit stage increments this to remove instructions
						   ///< from the list (FULL when head == tail)
						   ///<
		
		ROB_Entry* m_tail; ///< Dispatch stage increments this to add instructions
						   ///< to the list
						   ///<
						
		int rob_slot_counter;
		
		float* m_memory; ///< Pointer to hardware memory
		
		bool fp_adder_rs_in_use;
		bool fp_multiplier_rs_in_use;
		bool load_rs_in_use;
		bool store_rs_in_use;
		bool integer_rs_in_use;
		
		FP_Register m_fp_register[NUM_FP_REGISTERS]; ///< array of FP registers
		Int_Register m_int_register[NUM_INT_REGISTERS]; ///< array of integer registers
		std::vector<Reservation_Station> fp_multiplier_reservation_stations;
		std::vector<Reservation_Station> fp_adder_reservation_stations;
		std::vector<Reservation_Station> load_reservation_stations;
		std::vector<Reservation_Station> store_reservation_stations;
		std::vector<Reservation_Station> integer_reservation_stations;
		
		std::vector< std::vector<Reservation_Station>* > m_reservation_stations;
		
		std::vector<Instruction> m_instruction_queue;
		std::vector<Instruction>::iterator instruction_ptr; ///< next instruction 
															///< to be issued from queue
};

#endif // ROB_BUFFER_H_INCLUDED

