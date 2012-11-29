///////////////////////////////////////////////////////////////////////////////
/// This file implements Tamasulo's Algorithm with Reorder Buffer
///
/// @file rob_buffer.cpp
///////////////////////////////////////////////////////////////////////////////

#include "rob_buffer.hpp"

///////////////////////////////////////////////////////////////////////////////
static void process_reservation_station(Reservation_Station* rs, std::vector<Reservation_Station*>& waiting_units)
{
	if ((rs->m_waiting_rob_entry_value_one != NULL) &&
		!rs->m_waiting_rob_entry_value_one->m_busy)
	{
		rs->m_source_one = 
			rs->m_waiting_rob_entry_value_one->m_value;
		rs->m_waiting_rob_entry_value_one = NULL;
	}
	if ((rs->m_waiting_rob_entry_value_two != NULL) &&
		!rs->m_waiting_rob_entry_value_two->m_busy)
	{
		rs->m_source_two = 
			rs->m_waiting_rob_entry_value_two->m_value;
		rs->m_waiting_rob_entry_value_two = NULL;
	}
	if ((rs->m_waiting_rob_entry_value_one == NULL) && 
		(rs->m_waiting_rob_entry_value_two == NULL))
	{
		if (rs->m_execution_counter != 0)
		{
			rs->m_execution_counter--;
			rs->m_rob_entry->m_state = EXECUTING;
			std::cout<<rs->m_rob_entry->m_instruction->m_raw_instruction;
			std::cout<<"  "<<rs->m_execution_counter<<std::endl;
		}
		else // (rs->m_execution_counter == 0)
		{
			rs->m_rob_entry->m_state = EXECUTION_COMPLETE;
			
			// Perform respective operation to get result
			// rs->m_result...
			
			std::cout<<"Execution Complete: ";
			std::cout<<rs->m_rob_entry->m_instruction->m_raw_instruction<<std::endl;
		}
	}
	else
	{
		rs->m_rob_entry->m_state = WAITING;
		std::cout<<"Waiting: "<<rs->m_rob_entry->m_instruction->m_raw_instruction<<std::endl;
		waiting_units.push_back(rs);
	}
}

///////////////////////////////////////////////////////////////////////
bool ROB::reservation_station_available(std::vector<Instruction>::iterator& instruction, ROB_Entry*& rob_entry)
{	
	// Add instruction to operation unit vector
	bool reservation_station_available = false;
	
	if ((instruction->m_instruction == NOOP) &&
		(instruction->m_type != LOAD) && 
		(instruction->m_type != STORE))
	{
		return true;
	}
	
	Reservation_Station rs;
	rs.m_instruction = instruction->m_instruction;
	rs.m_rob_entry = rob_entry;
	
	switch(instruction->m_type)
	{
		case FLOATING_POINT:
			if ((instruction->m_instruction == MULTIPLY 
				|| instruction->m_instruction == DIVIDE)
					&& !fp_multiplier_rs_in_use)
			{
				if (fp_multiplier_reservation_stations.size() != NUM_FP_MULTIPLIER_RS)
				{
				 	rs.m_execution_counter = 
						(instruction->m_instruction == 
							MULTIPLY ? FP_MULTIPLY_CYCLE_TIME: FP_DIVIDE_CYCLE_TIME);
					
					fp_multiplier_rs_in_use = true;
					reservation_station_available = true;
				}
				else
				{
					break;
				}
			}
			else // m_instruction == ADD || m_instruction == SUBTRACT
			{
				if ((fp_adder_reservation_stations.size() != NUM_FP_ADDER_RS)
					&& !fp_adder_rs_in_use)
				{
					rs.m_execution_counter = FP_ADD_SUB_CYCLE_TIME;
						
					fp_adder_rs_in_use = true;	
					reservation_station_available = true;
				}
				else
				{
					break;
				}
			}
			
			goto SETUP;
			
		case INTEGER:
			if ((integer_reservation_stations.size() != NUM_INT_RS)
				&& !integer_rs_in_use)
			{
				rs.m_execution_counter = INTEGER_INSTRUCTION_CYCLE_TIME;
				integer_reservation_stations.push_back(rs);
				integer_rs_in_use = true;
				reservation_station_available = true;
				
				goto SETUP;
			}
			
			break;
			
		case LOAD:
			if ((load_reservation_stations.size() != NUM_LOAD_RS)
				&& !load_rs_in_use)
			{
				rs.m_execution_counter = LOAD_CYCLE_TIME;
				load_reservation_stations.push_back(rs);
				load_rs_in_use = true;
				reservation_station_available = true;
		
				goto SETUP;
			}
			
			break;
			
		case STORE:
			if ((store_reservation_stations.size() != NUM_STORE_RS)
				&& !store_rs_in_use)
			{
				rs.m_execution_counter = STORE_CYCLE_TIME;
				store_reservation_stations.push_back(rs);
				store_rs_in_use = true;
				reservation_station_available = true;
				
				goto SETUP;
			}
			
			break;
			
		default:
			std::cout<<"Invalid instruction type"<<std::endl;
			std::cout<<instruction->m_type<<std::endl;
			assert(false);
	} 

	goto DONE;
	
SETUP:
    if (instruction->m_source_register_one == NULL) // LD, ST, etc
	{
		rs.m_source_one = instruction->m_load_store_offset;
		assert((instruction->m_type == LOAD) ||
			(instruction->m_type == STORE));
	}
	else if (instruction->m_source_register_one->m_busy)
	{
		// Store the rob_entry containing the pending result
		rs.m_waiting_rob_entry_value_one = 
			instruction->m_source_register_one->m_rob_entry;
	}
	else
	{
		rs.m_source_one = instruction->m_source_register_one->m_data;
	}
	
	if (instruction->m_source_register_two == NULL) // ADDI, SUBI, etc
	{
		rs.m_source_two = instruction->m_immediate_value;
	}
	else if (instruction->m_source_register_two->m_busy)
	{
		// Store the rob_entry containing the pending result
		rs.m_waiting_rob_entry_value_two = 
			instruction->m_source_register_two->m_rob_entry;
	}
	else
	{
		rs.m_source_two = instruction->m_source_register_two->m_data;
	}
	
	if (instruction->m_type == FLOATING_POINT)
	{
		if ((instruction->m_instruction == MULTIPLY) || 
			(instruction->m_instruction == DIVIDE))
		{
			fp_multiplier_reservation_stations.push_back(rs);
		}
		else
		{
			fp_adder_reservation_stations.push_back(rs);
		}
	}

DONE:
	return reservation_station_available;
}

///////////////////////////////////////////////////////////////////////
void ROB::execute_intructions(void)
{
	bool writing = false; // only one write per cycle
	
	std::vector<Reservation_Station*> waiting_units;
	
	// Loop through all the reservation stations
	for (typeof(m_reservation_stations.begin()) it = m_reservation_stations.begin(); it < m_reservation_stations.end(); it++)
	{	
		std::vector<typeof((*it)->begin())> slots_to_remove;
		// Loop through all the slots in this reservation station
		for (typeof((*it)->begin()) rs = (*it)->begin(); rs < (*it)->end(); rs++)
		{
			// Write results
			if ((rs->m_rob_entry->m_state == EXECUTION_COMPLETE) &&
				(writing == false))
			{
				std::cout<<"Writing Result for instruction: ";
				std::cout<<rs->m_rob_entry->m_instruction->m_raw_instruction<<std::endl;
				writing = true;
				rs->m_rob_entry->m_value = rs->m_result;
				rs->m_rob_entry->m_busy = false;
				slots_to_remove.push_back(rs);
				continue;
			}
			else
			{
				process_reservation_station(&(*rs), waiting_units);
				std::cout<<"Num of Waiting: "<<waiting_units.size()<<std::endl;
			}
		}
		
		// Remove all the completed reservation station units
		for(typeof(slots_to_remove.begin()) it_two = slots_to_remove.begin(); 
			           it_two < slots_to_remove.end(); it_two++)
		{
			(*it)->erase(*it_two);
		}
		
		// Check the waiting units and see if they have 
		for (typeof(waiting_units.begin()) waiting_rs = waiting_units.begin();
			waiting_rs < waiting_units.end(); waiting_rs++)
		{
			
		}
		
	}
}

///////////////////////////////////////////////////////////////////////
ROB::ROB(void)
{
	m_head = NULL;
	ROB_Entry* last;
	ROB_Entry* new_node;
	
	// Build doublely linked list of ROB Entries
	for (unsigned int i=0; i<ROB_SIZE; i++)
	{
		// Allocate new entry
		new_node = new ROB_Entry();
		
		if (m_head == NULL)
		{
			// Insert head of list
			m_head = new_node;
			last = new_node;
		}
		else
		{
			// Insert new entry
			last->m_next = new_node;
			last = new_node;
			
			// Point last entry to head
			last->m_next = m_head;
		}
		
		// Verify correct entry numbers are stored
		assert(last->m_entry_number == i);
	}
	
	// Initialize the tail pointer (available ROB slot)
	m_tail = m_head;
	
	rob_slot_counter = 0;
	
	// Store all the reservation stations in a list to be iterated later
	m_reservation_stations.push_back(&fp_multiplier_reservation_stations);
	m_reservation_stations.push_back(&fp_adder_reservation_stations);
	m_reservation_stations.push_back(&load_reservation_stations);
	m_reservation_stations.push_back(&store_reservation_stations);
	m_reservation_stations.push_back(&integer_reservation_stations);
	
	std::string instruction;
	std::ifstream myfile("instructions.txt");
	if (myfile.is_open())
	{	
		std::stringstream ss;
		std::string operation;
		std::string destination;
		std::string source1;
		std::string source2;
		std::string temp;
		
		while(myfile.good())
		{
			Instruction temp_instruction;
			
			getline(myfile, instruction);
			
			temp_instruction.m_raw_instruction = instruction;
			
			ss.str("");
			ss.clear();
			
			ss << instruction;

			ss >> operation;
			
			if (std::find(operation.begin(), operation.end(), ':') != operation.end())
			{
				ss.str("");
				ss.clear();
				std::string loop_name;
				
				ss << instruction;
				ss >> loop_name >> operation;
				
				loop_name.erase(loop_name.end()-1); // remove ":"
			}
			
			if (operation == "LD" || operation == "SD")
			{
				ss >> destination >> temp;
				
				source1 = std::string(temp.begin(), 
								std::find(temp.begin(), temp.end(), '('));
				source2 = std::string(
								std::find(temp.begin(), temp.end(), '(')+1, 
								std::find(temp.begin(), temp.end(), ')')
								);	
				
				// Create Instruction object
				temp_instruction.m_type = (operation == "LD" ? LOAD : STORE);
				
				// Store Destination Register
				if (destination[0] == 'F')
				{
					std::string des_register(destination.begin()+1, destination.end());
					temp_instruction.m_destination_register = 
						m_fp_register + atoi(des_register.c_str());
				}
				else if (destination[0] == 'R')
				{
					std::string des_register(destination.begin()+1, destination.end());
					temp_instruction.m_destination_register = 
						m_int_register + atoi(des_register.c_str());
				}
				else
				{
					throw std::runtime_error("Invalid register for load/store");
				}
				
				// Store memory index for load/store (source 1)
				temp_instruction.m_load_store_offset = atoi(source1.c_str());
				
				// Store source register
				if (source2[0] == 'R')
				{
					std::string source_register(source2.begin()+1, source2.end());
					temp_instruction.m_source_register_two = 
						m_int_register + atoi(source_register.c_str());
				}
				else if (source2[0] == 'F')
				{
					std::string source_register(source2.begin()+1, source2.end());
					temp_instruction.m_source_register_two = 
						m_fp_register + atoi(source_register.c_str());
				}
				else
				{
					throw std::runtime_error("Invalid register for load/store");
				}
				
				// Add this instruction to the queue
				m_instruction_queue.push_back(temp_instruction);
			}
			else if (operation == "BNEQ" || operation == "BNEZ" || operation == "BEQ")
			{
				ss >> source1 >> source2 >> destination;
			}
			else if (operation == "NOOP")
			{
				temp_instruction.m_instruction = NOOP;
				// Add this instruction to the queue
				m_instruction_queue.push_back(temp_instruction);
				continue;
			}
			else
			{
				ss >> destination >> source1 >> source2;
				
				// Create Instruction object
				
				if ((operation == "ADDD") || (operation == "ADD"))
				{
					temp_instruction.m_instruction = ADD;
					temp_instruction.m_type = (operation == "ADDD" ? FLOATING_POINT : INTEGER);
				}
				else if ((operation == "SUB") || (operation == "SUBD"))
				{
					temp_instruction.m_instruction = SUBTRACT;
					temp_instruction.m_type = (operation == "SUBD" ? FLOATING_POINT : INTEGER);
				}
				else if ((operation == "MULT") || (operation == "MULTD"))
				{
					temp_instruction.m_instruction = MULTIPLY;
					temp_instruction.m_type = (operation == "MULTD" ? FLOATING_POINT : INTEGER);
				}
				else if ((operation == "DIV") || (operation == "DIVD"))
				{
					temp_instruction.m_instruction = DIVIDE;
					temp_instruction.m_type = (operation == "DIVD" ? FLOATING_POINT : INTEGER);
				}
				else if (operation == "ADDI")
				{
					temp_instruction.m_instruction = ADDI;
					temp_instruction.m_type = INTEGER;
				}
				else if (operation == "SUBI")
				{
					temp_instruction.m_instruction = SUBI;
					temp_instruction.m_type = INTEGER;
				}
				else
				{
					throw std::runtime_error("Invalid operation: " + operation);
				}
				
				// Store Destination Register
				if (destination[0] == 'F')
				{
					std::string des_register(destination.begin()+1, destination.end());
					temp_instruction.m_destination_register = 
						m_fp_register + atoi(des_register.c_str());
				}
				else if (destination[0] == 'R')
				{
					std::string des_register(destination.begin()+1, destination.end());
					temp_instruction.m_destination_register = 
						m_int_register + atoi(des_register.c_str());
				}
				else
				{
					assert(false);
					throw std::runtime_error("Invalid instruction file format");
				}
				
				// Store source 1 register
				if (source1[0] == 'R')
				{
					std::string source_register(source1.begin()+1, source1.end());
					temp_instruction.m_source_register_one = 
						m_int_register + atoi(source_register.c_str());
				}
				else if (source2[0] == 'F')
				{
					std::string source_register(source1.begin()+1, source1.end());
					temp_instruction.m_source_register_one = 
						m_fp_register + atoi(source_register.c_str());
				}
				
				// Store source 2 register
				if (source2[0] == 'R')
				{
					std::string source_register(source2.begin()+1, source2.end());
					temp_instruction.m_source_register_two = 
						m_int_register + atoi(source_register.c_str());
				}
				else if (source2[0] == 'F')
				{
					std::string source_register(source2.begin()+1, source2.end());
					temp_instruction.m_source_register_two = 
						m_fp_register + atoi(source_register.c_str());
				}
				else if (operation == "ADDI" || operation == "SUBI")
				{
					temp_instruction.m_immediate_value = atoi(source2.c_str());
				}
				else
				{
					throw std::runtime_error("Invalid instruction file format");
				}
				
				// Add this instruction to the queue
				m_instruction_queue.push_back(temp_instruction);
			}
		}
		myfile.close();
	}
	else
	{
		throw std::runtime_error("Could not open instruction file");
	}
	
	instruction_ptr = m_instruction_queue.begin();
}

///////////////////////////////////////////////////////////////////////
ROB::~ROB(void)
{
	ROB_Entry* node = m_head;
	ROB_Entry* next_node;
	
	// Delete the memory allocated for each
	// reorder buffer entry
	for (unsigned int i=0; i<ROB_SIZE; i++)
	{
		next_node = node->m_next;
		delete node;
		node = next_node;
	}
}

///////////////////////////////////////////////////////////////////////
void ROB::issue_instruction(void)
{	
	fp_adder_rs_in_use = false;
	fp_multiplier_rs_in_use = false;
	load_rs_in_use = false;
	store_rs_in_use = false;
	integer_rs_in_use = false;
	
	int num_of_issued_instructions = 0;
	
	// Check if ROB has a free entry
	// if reservation station available....
	// Adds instruction to reservation station
	
	while ((rob_slot_counter != ROB_SIZE) &&
		   (instruction_ptr != m_instruction_queue.end()) &&
		   (num_of_issued_instructions < MAX_ISSUE_PER_CYCLE) &&
		    (reservation_station_available(instruction_ptr, m_tail)))
	{
		// Insert instruction in reservation station
		
		// increment issue count
		num_of_issued_instructions++;
		
		// Initialize entry parameters
		m_tail->m_instruction = instruction_ptr;
		instruction_ptr++;
		
		m_tail->m_state = ISSUE;
		m_tail->m_destination_register = m_tail->m_instruction->m_destination_register;
		m_tail->m_destination_register->m_busy = true;

		// Store entry pointer in destination register
		// so any following instruction issued knows to check this 
		// entry for the result if/when it needs it
		m_tail->m_destination_register->m_rob_entry = m_tail;
		
		// Point tail to next entry to be filled
		m_tail = m_tail->m_next;

		// Increment entry counter
		rob_slot_counter++;
	}
}

///////////////////////////////////////////////////////////////////////
void ROB::commit_instruction(ROB_Entry*& rob_entry)
{
	rob_entry->m_state = EMPTY;
	rob_slot_counter--;
	this->m_head = this->m_head->m_next;
}

///////////////////////////////////////////////////////////////////////
void ROB::process_instructions(void)
{
	// COMMIT
	// Attempt to commit instruction waiting to be commited.
	// Only one commit per cycle.
	if (m_head->m_state == COMMIT)
	{
		commit_instruction(m_head); //removes from ROB
	}
	
	// EXECUTE
	execute_intructions(); 
}

// Initialize the ROB Entry numbers
unsigned int ROB_Entry::s_next_id = 0;

