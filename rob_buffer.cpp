///////////////////////////////////////////////////////////////////////////////
/// This file implements Tamasulo's Algorithm with Reorder Buffer
///
/// @file rob_buffer.cpp
///////////////////////////////////////////////////////////////////////////////

#include "rob_buffer.hpp"

//Instruction_Set m_instruction;
//Register* m_destination_register;
//Register* m_source_register_one;
//Register* m_source_register_two;
//unsigned int m_execution_counter;

///////////////////////////////////////////////////////////////////////
/// Check is a reservation station is available for this instruction
///
/// @param[in] instruction The instruction to be executed
///
/// @returns TRUE if a reservation station is available
///////////////////////////////////////////////////////////////////////
static bool reservation_station_available(Instruction& instruction)
{
	bool reservation_station_available = false;
	
	//switch(instruction.m_instruction)
	//{
		
	//} 
	
	return reservation_station_available;
}

///////////////////////////////////////////////////////////////////////
/// Execute the instruction in the RS corresponding to a ROB entry
///
/// @param[in] rob_entry rob entry containing instruction executing in RS
///////////////////////////////////////////////////////////////////////
static void execute_intruction(ROB_Entry*& rob_entry)
{
	
}

///////////////////////////////////////////////////////////////////////
/// Write the result of the instruction to the common data bus
///
/// @param[in] rob_entry rob entry waiting on reservation station result
///////////////////////////////////////////////////////////////////////
static void write_instruction(ROB_Entry*& rob_entry)
{
	
}

///////////////////////////////////////////////////////////////////////
ROB::ROB(void)
{
	m_head = NULL;
	ROB_Entry* last;
	ROB_Entry* new_node;
	m_rob_head = NULL;
	
	// Build doublely linked list of ROB Entries
	for (unsigned int i=0; i<ROB_SIZE; i++)
	{
		// Allocate new entry
		new_node = new ROB_Entry();
		
		if (m_rob_head == NULL)
		{
			// Insert head of list
			m_rob_head = new_node;
			last = new_node;
		}
		else
		{
			// Insert new entry
			last->m_next = new_node;
			last = new_node;
			
			// Point last entry to head
			last->m_next = m_rob_head;
		}
		
		// Verify correct entry numbers are stored
		assert(last->m_entry_number == i);
	}
	
	// Initialize the tail pointer (available ROB slot)
	m_tail = m_rob_head;
}

///////////////////////////////////////////////////////////////////////
ROB::~ROB(void)
{
	ROB_Entry* node = m_head;
	ROB_Entry* next_node;
	
	// Delete the memory allocated for each
	// reorder buffer entry
	while (node != NULL)
	{
		next_node = node->m_next;
		delete node;
		node = next_node;
	}
}
///////////////////////////////////////////////////////////////////////
bool ROB::issue_instruction(Instruction issued_instruction)
{
	bool issue_successful = false;
	
	// Check if ROB has a free entry and reservation station is available
	if ((m_tail != m_head) && reservation_station_available(issued_instruction))
	{
		if (m_head == NULL)
		{
			// The ROB is empty
			m_head = m_rob_head;
		}
		
		// Initialize entry parameters
		m_tail->m_instruction = issued_instruction;
		m_tail->m_state = ISSUE;
		m_tail->m_busy = true;
		
		// Initialize instruction counter
		switch (m_tail->m_instruction.m_instruction)
		{
			case MULTIPLY:
				m_tail->m_instruction.m_execution_counter = 2;
				break;
			case DIVIDE:
				m_tail->m_instruction.m_execution_counter = 2;
				break;
			case ADD:
				m_tail->m_instruction.m_execution_counter = 2;
				break;
			case SUBTRACT:
				m_tail->m_instruction.m_execution_counter = 2;
				break;
			default:
				std::cout<<"Invalid instruction"<<std::endl;	
				assert(false);
		}
		
		// Point tail to next entry to be filled
		m_tail = m_tail->m_next;
		
		issue_successful = true;
	}
	
	return issue_successful;
}

///////////////////////////////////////////////////////////////////////
bool ROB::commit_instruction(ROB_Entry*& rob_entry)
{
	return true;
}

///////////////////////////////////////////////////////////////////////
void ROB::process_instructions(void)
{
	// Get the first ROB entry
	ROB_Entry* temp = m_head;
	
	// Only one instruction can be committed per cycle
	bool instruction_committing = false;
	
	// Only one write can be done per cycle
	bool instruction_writing = false;
	
	do
	{	
		// COMMIT
		// Attempt to commit instruction waiting to be commited.
		// Only one commit per cycle.
		if ((temp->m_state == COMMIT) && 
				(instruction_committing == false))
		{
			instruction_committing = true;
			commit_instruction(temp);
			temp->m_state = EMPTY;
		}
		
		// WRITE
		// Attempt to write registers that are done executing.
		// Only one write can be done per cycle.
		else if ((temp->m_state == EXECUTION_COMPLETE) &&
				(instruction_writing == false))
		{
			instruction_writing = true;
			write_instruction(temp);
			temp->m_state = COMMIT;
		}
		
		// EXECUTE
		// Execute instructions if registers available
		else if ((temp->m_state == EXECUTING) ||
					temp->m_state == ISSUE)
		{
			execute_intruction(temp);
			
			// Check if now done executing
			if (temp->m_instruction.m_execution_counter == 0)
			{
				temp->m_state = EXECUTION_COMPLETE;
			}
		}
		
		temp = temp->m_next;
	} while(temp != m_head);
	
	// Begin execution of waiting units if data is now available
}

// Initialize the ROB Entry numbers
unsigned int ROB_Entry::s_next_id = 0;

