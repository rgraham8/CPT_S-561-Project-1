///////////////////////////////////////////////////////////////////////////////
/// This file implements Tamasulo's Algorithm with Reorder Buffer
///////////////////////////////////////////////////////////////////////////////

#include "rob_buffer.hpp"

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
	
	// Execute waiting instructions
}