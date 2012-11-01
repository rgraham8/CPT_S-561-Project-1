///////////////////////////////////////////////////////////////////////////////
/// This file implements Tamasulo's Algorithm with Reorder Buffer
///////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <iostream>
#include <assert.h>

#define ROB_SIZE 32 // number of entries in the Reorder buffer

// List of the available instructions
enum INSTRUCTION
{
	NA,
	ADD,
	MULT,
	DIV
};

// The states of an ROB entry
enum STATE
{
	EMPTY,
	ISSUE,
	EXECUTE,
	COMMIT
};

///////////////////////////////////////////////////////////////////////////////
// One entry in the ROB 
///////////////////////////////////////////////////////////////////////////////
class ROB_Entry
{
	public:
		bool m_busy; // whether unit is busy
		INSTRUCTION m_instruction; // instruction being issued/executed
		STATE m_state; // the state of the unit
		long* m_destination; // the destination register
		unsigned int m_entry_number; // entry number
		ROB_Entry* m_next; // next pointer for circular buffer
		
		///////////////////////////////////////////////////////////////////////
		// Constructor
		///////////////////////////////////////////////////////////////////////
		ROB_Entry(void):	
			m_busy(false),
			m_instruction(NA),
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
		ROB(void)
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
		/// Issue an instruction the to reorder buffer
		///
		/// @param[in] issued_instruction the instruction to add to the ROB
		///
		/// @returns FALSE if a slot was not available in the ROB. TRUE otherwise
		///////////////////////////////////////////////////////////////////////
		bool issue_instruction(INSTRUCTION issued_instruction)
		{
			bool issue_successful = false;
			
			// Check if ROB has a free entry
			if (m_tail != m_head)
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
				
				// Point tail to next entry to be filled
				m_tail = m_tail->m_next;
				
				issue_successful = true;
			}
			
			return issue_successful;
		}
		
		bool execute_instruction();
		bool commit_instruction();
		
	private:
		ROB_Entry* m_rob_head; // head of ROB buffer
		
		// Commit stage increments this to remove instructions
		// from the list (FULL when head == tail)
		ROB_Entry* m_head;
		
		// Dispatch stage increments this to add instructions
		// to the list
		ROB_Entry* m_tail;
};

// Initialize the ROB Entry numbers
unsigned int ROB_Entry::s_next_id = 0;

///////////////////////////////////////////////////////////////////////////////
/// Program entry point
///
/// @param[in] argc number command-line of args
/// @param[in] argv array of command-line arguments
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // Initialize the ROB buffer
	ROB reorder_buffer;

	return 0;
}