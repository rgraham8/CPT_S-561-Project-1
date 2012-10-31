// This file implements Tamasulo's Algorithm with Reorder Buffer
#include <stdint.h>
#include <iostream>
#include <assert.h>

#define ROB_SIZE 32

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

// One entry in the ROB 
class ROB_Entry
{
	public:
		bool m_busy;
		INSTRUCTION m_instruction;
		STATE m_state;
		long* m_destination;
		unsigned int m_entry_number; // entry number
		
		///////////////////////////////////////////////////////////////////////
		// Constructor
		///////////////////////////////////////////////////////////////////////
		ROB_Entry(void):
			m_busy(false),
			m_instruction(NA),
			m_state(EMPTY),
			m_destination(NULL)
   		{
			m_entry_number = ROB_Entry::s_next_id++;
		}
	
	private:
		ROB_Entry* next;
		static unsigned int s_next_id; // used to increment each entry number
};

class ROB
{
	public:
		ROB(void)
		{
			m_start = &m_buffer[0];
			m_end = m_start;
		}
		
		ROB_Entry& get_entry(const unsigned int index)
		{
			return m_buffer[index];
		}
	private:
		ROB_Entry* m_start;
		ROB_Entry* m_end;
		ROB_Entry m_buffer[ROB_SIZE];
};

// Initialize the ROB Entry number 
unsigned int ROB_Entry::s_next_id = 0;


int main(int argc, char* argv[])
{
    // Initialize the ROB buffer
	ROB reorder_buffer;
	
	ROB_Entry x = reorder_buffer.get_entry(2);
	
	std::cout<<x.m_entry_number<<std::endl;

	return 0;
}