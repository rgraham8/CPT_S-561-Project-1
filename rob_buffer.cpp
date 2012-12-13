///////////////////////////////////////////////////////////////////////////////
/// This file implements Tamasulo's Algorithm with Reorder Buffer
///
/// @file rob_buffer.cpp
///////////////////////////////////////////////////////////////////////////////

#include "rob_buffer.hpp"

///////////////////////////////////////////////////////////////////////////////
void ROB::process_reservation_station(Reservation_Station* rs, std::vector<Reservation_Station*>& waiting_units, bool from_queue)
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
			
			if (rs->m_rob_entry->m_instruction->m_instruction == BNEZ)
			{
				if (rs->m_source_one == 0)
				{
					branch_predictor[rs->m_rob_entry->m_instruction->m_raw_instruction] = false;
					m_flush_buffer = true;
					m_branch_entry = rs->m_rob_entry;
				}
			}
		}
		else // (rs->m_execution_counter == 0)
		{
			if (rs->m_rob_entry->m_state == EXECUTION_COMPLETE)
			{
				return;
			}
			
			rs->m_rob_entry->m_state = EXECUTION_COMPLETE;
			
			float* temp = NULL;
			Register* temp_register;
			
			// Perform respective operation to get result
			switch(rs->m_rob_entry->m_instruction->m_type)
			{
				case FLOATING_POINT:
				case INTEGER:
					switch(rs->m_rob_entry->m_instruction->m_instruction)
					{
						case ADD:
						case ADDI:
							rs->m_result = rs->m_source_one + rs->m_source_two;
							//assert(false);
							
							//assert(false);
							break;
						case MULTIPLY:
							rs->m_result = rs->m_source_one * rs->m_source_two;
							break;
						case DIVIDE:
							rs->m_result = rs->m_source_one / rs->m_source_two;
							break;
						case SUBTRACT:
						case SUBI:
							rs->m_result = rs->m_source_one - rs->m_source_two;
							break;	
						case SLTI:
							rs->m_result = (rs->m_source_one < rs->m_source_two) ? 1 : 0;
							break;
						default:
							assert(false);
					}
					
					if (rs->m_rob_entry->m_instruction->m_instruction != BNEZ)
					{
						// Cast to float or int
						rs->m_result = (rs->m_rob_entry->m_instruction->m_type == INTEGER) ? (int)rs->m_result : (float)rs->m_result;
					}
					break;
					
				case LOAD:
					//std::cout<<rs->m_rob_entry->m_instruction->m_raw_instruction;
					//std::cout<<" "<<rs->m_source_one<<" "<<rs->m_source_two<<std::endl;
					
					temp = (m_memory + (int)rs->m_source_one + (int)(rs->m_source_one));
					
					//temp = &temp_register->m_data;
					assert(temp <= &m_memory[MEMORY_SIZE_BYTES-1]);
					
					rs->m_result = *temp;
					
					// Cast to float or int
					rs->m_result = (rs->m_rob_entry->m_destination_register->m_type == INT) ? (int)rs->m_result : (float)rs->m_result;
					
					break;
					
				case STORE:
					temp = (m_memory + (int)rs->m_rob_entry->m_instruction->m_load_store_offset + (int)rs->m_source_two);
					*temp = rs->m_source_two;
					assert(temp <= &m_memory[MEMORY_SIZE_BYTES-1]);
					
					break;
					
				default:
					assert(rs->m_rob_entry->m_instruction->m_type == BRANCH);
					
			}
		}
	}
	else if (!from_queue)
	{
		rs->m_rob_entry->m_state = WAITING;

		if (rs->m_waiting_rob_entry_value_one != NULL)
		{
			assert(rs->m_rob_entry != rs->m_waiting_rob_entry_value_one);
			//std::cout<<rs->m_waiting_rob_entry_value_one->m_entry_number;
		}
		if (rs->m_waiting_rob_entry_value_two != NULL)
		{
			assert(rs->m_rob_entry != rs->m_waiting_rob_entry_value_two);
		}
		//assert(false);

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
		return false;
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
		case BRANCH:
		case INTEGER:
			if ((integer_reservation_stations.size() != NUM_INT_RS)
				&& !integer_rs_in_use)
			{
				rs.m_execution_counter = INTEGER_INSTRUCTION_CYCLE_TIME;
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
    if (instruction->m_source_register_one == NULL) // LD, etc
	{
		rs.m_source_one = instruction->m_load_store_offset;
		if (instruction->m_type != LOAD)
		{
			std::cout<<"ERROR: "<<instruction->m_raw_instruction<<std::endl;
			assert(false);
		}
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
		if (instruction->m_type != BRANCH)
		{
			rs.m_source_two = instruction->m_immediate_value;
		}
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
	else if ((instruction->m_type == INTEGER) 
			|| (instruction->m_type == BRANCH))
	{
		integer_reservation_stations.push_back(rs);
	}
	else if (instruction->m_type == LOAD)
	{
		load_reservation_stations.push_back(rs);
	}
	else if (instruction->m_type == STORE)
	{
		store_reservation_stations.push_back(rs);
	}

DONE:
	return reservation_station_available;
}

///////////////////////////////////////////////////////////////////////
void ROB::execute_intructions(void)
{
	int writing = 0; // 4 write per cycle
	
	std::vector<Reservation_Station*> waiting_units;
	
	// Loop through all the reservation stations
	for (int i=0; i < m_reservation_stations.size(); i++)
	{	
		// Loop through all the slots in this reservation station
		for (int j = 0; j < m_reservation_stations[i]->size(); j++)
		{
			Reservation_Station* rs = &(m_reservation_stations[i]->at(j));
			
			// Write results
			if ((rs->m_rob_entry->m_state == EXECUTION_COMPLETE))
			{
				if (rs->m_rob_entry->m_instruction->m_instruction == ADDI)
				{
					//std::cout<<rs->m_rob_entry->m_instruction->m_raw_instruction<<std::endl;
					assert(rs->m_rob_entry->m_busy == true);
				}
				
				if(first_write_cycle_ < 0)
				{
					first_write_cycle_ = m_cycle_number;
				}
				
				if (writing == m_max_num_issue)
				{
					continue;
				}

				writing++;
				//std::cout<<"Writing: "<<rs->m_rob_entry->m_instruction->m_raw_instruction<<std::endl;
				rs->m_rob_entry->m_value = rs->m_result;
				rs->m_rob_entry->m_busy = false;
				rs->m_rob_entry->m_state = COMMIT;
				m_reservation_stations[i]->erase(m_reservation_stations[i]->begin() + j);
				j--;
				continue;
			}
			else
			{
				process_reservation_station(&(*rs), waiting_units, false);
			}
		}
	}
	
	// Check the waiting units and see if they have 
	for (typeof(waiting_units.begin()) waiting_rs = waiting_units.begin();
		waiting_rs != waiting_units.end(); waiting_rs++)
	{
		process_reservation_station((*waiting_rs), waiting_units, true);
	}
	
	m_num_of_writes_per_cycle.push_back(writing);
}

///////////////////////////////////////////////////////////////////////
ROB::ROB(void)
{
	m_head = NULL;
	ROB_Entry* last;
	ROB_Entry* new_node;
	first_write_cycle_ = -1;
	last_issue_cycle_ = 1;
	
	// Init Registers
	m_int_register[0].m_data = 0;
	m_int_register[1].m_data = 68;
	m_int_register[2].m_data = 88;
	m_int_register[3].m_data = 3;

	m_max_num_issue = 1;
	m_flush_buffer = false;
	m_cycle_number = 0;
	
	// Create memory
	m_memory = new float[MEMORY_SIZE_BYTES];
	srand (time(NULL));
	for (int i = 0; i<MEMORY_SIZE_BYTES; ++i)
	{
		m_memory[i] = rand() % 100;
	}
	
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
				
				temp_instruction.m_branch_name = loop_name;
				
				// Store the instruction without the LOOP_NAME
				temp_instruction.m_raw_instruction =  
					std::string(std::find(instruction.begin(), instruction.end(), ':')+2, instruction.end());
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
				
				if (operation == "SD")
				{	
					// Store memory offset for load/store
					temp_instruction.m_load_store_offset = atoi(source1.c_str());
					
				}
				else
				{
					// Store memory offset for load/store
					temp_instruction.m_load_store_offset = atoi(source1.c_str());
				}
				
				// Create Instruction object
				temp_instruction.m_type = (operation == "LD" ? LOAD : STORE);
				
				// Store Destination Register
				if ((operation == "LD") || (operation == "SD"))
				{
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
				}
				
				// For store, the source register 1 is the register to write to the memory
				if (temp_instruction.m_type == STORE)
				{
					temp_instruction.m_source_register_one = temp_instruction.m_destination_register;
					temp_instruction.m_destination_register = NULL;
				}
				
				// Store source register (source 2)
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
				ss >> source1 >> destination;
				if (operation == "BNEZ")
				{
					if (branch_predictor.find(temp_instruction.m_raw_instruction) == branch_predictor.end()){}
					{
						//std::cout<<temp_instruction.m_raw_instruction<<std::endl;
						branch_predictor[temp_instruction.m_raw_instruction] = true;
					}
					
					if (source1[0] == 'R')
					{
						std::string source_register(source1.begin()+1, source1.end());
						temp_instruction.m_source_register_one = 
							m_int_register + atoi(source_register.c_str());
					}
					else if (source1[0] == 'F')
					{
						std::string source_register(source2.begin()+1, source2.end());
						temp_instruction.m_source_register_one = 
							m_fp_register + atoi(source_register.c_str());
					}
					temp_instruction.m_type = BRANCH;
					temp_instruction.m_instruction = BNEZ;
					temp_instruction.m_loop_name = destination;
					m_instruction_queue.push_back(temp_instruction);
				}
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
				else if (operation == "SLTI")
				{
					temp_instruction.m_instruction = SLTI;
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
				else if (source1[0] == 'F')
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
				else if (operation == "ADDI" || operation == "SUBI" || operation == "SLTI")
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
	
	delete [] m_memory;
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
		   (num_of_issued_instructions < m_max_num_issue))
	{
		if (!reservation_station_available(instruction_ptr, m_tail))
		{
			break;
		}
		// Insert instruction in reservation station
		
		// increment issue count
		num_of_issued_instructions++;
		
		// Initialize entry parameters
		m_tail->m_instruction = instruction_ptr;

		
		if (m_tail->m_instruction->m_instruction == BNEZ && 
			branch_predictor[m_tail->m_instruction->m_raw_instruction])
		{
			std::vector<Instruction>::iterator it = m_instruction_queue.begin();
			while (it != m_instruction_queue.end())
			{
				if (it->m_branch_name == m_tail->m_instruction->m_loop_name)
				{
					instruction_ptr = it;
					break;
				}
				it++;
			}
			assert(it != m_instruction_queue.end());
			assert(m_tail->m_state == EMPTY);
			last_issue_cycle_ = m_cycle_number;
			m_tail->m_state = ISSUE;
		}
		else
		{
			instruction_ptr++;
			assert(m_tail->m_state == EMPTY);
			m_tail->m_state = ISSUE;
			last_issue_cycle_ = m_cycle_number;
		}
		
		if (m_tail->m_instruction->m_destination_register == NULL)
		{
			assert((m_tail->m_instruction->m_type == STORE) || 
				(m_tail->m_instruction->m_instruction == BNEZ));
		}
		else
		{
			m_tail->m_destination_register = m_tail->m_instruction->m_destination_register;
		
			// Store entry pointer in destination register
			// so any following instruction issued knows to check this 
			// entry for the result if/when it needs it
			m_tail->m_destination_register->m_rob_entry = m_tail;
			m_tail->m_destination_register->m_busy = true;
		}
		
		// Set ROB entry to busy
		m_tail->m_busy = true;
		
		// Point tail to next entry to be filled
		m_tail = m_tail->m_next;

		// Increment entry counter
		rob_slot_counter++;
	}
	
	m_num_of_issues_per_cycle.push_back(num_of_issued_instructions);
}

///////////////////////////////////////////////////////////////////////
void ROB::commit_instruction(void)
{
	int num_commit = 0;
	while(num_commit != m_max_num_issue)
	{
		if (m_head->m_state == COMMIT)
		{
			assert(m_head->m_busy == false);
			if ((m_head->m_instruction->m_type != STORE) 
				&& (m_head->m_instruction->m_type != BRANCH))
			{
				m_head->m_destination_register->m_data = m_head->m_value;
				
				if (m_head->m_destination_register->m_rob_entry == m_head)
				{
					m_head->m_destination_register->m_rob_entry = NULL;
				}
			}
			
			m_head->m_state = EMPTY;
			rob_slot_counter--;
			m_head = m_head->m_next;
		}
		else
		{
			break;
		}
		num_commit++;
	}

	m_num_of_commits_per_cycle.push_back(num_commit);
	
}

///////////////////////////////////////////////////////////////////////
bool ROB::process_instructions(void)
{	
	std::cout<<"\n<-- Cycle #: "<<m_cycle_number+1<<" -->\n"<<std::endl;
	
	commit_instruction();
	execute_intructions();
	issue_instruction();
	
	if (m_flush_buffer)
	{
		m_flush_buffer = false;
		//assert(false);
		
		ROB_Entry* entry_to_remove = m_branch_entry->m_next;
		
		if (entry_to_remove != m_head)
		{
			m_tail = entry_to_remove;
		}
		
		instruction_ptr = m_branch_entry->m_instruction+1;

		while (entry_to_remove != m_head)
		{	
			bool done_removing = false;
			// Loop through all the reservation stations
			for (int i=0; i < m_reservation_stations.size(); i++)
			{	
				// Loop through all the slots in this reservation station
				for (int j = 0; j < m_reservation_stations[i]->size(); j++)
				{
					Reservation_Station* rs = &(m_reservation_stations[i]->at(j));

					// Remove Slot
					if ((rs->m_rob_entry == entry_to_remove))
					{
						m_reservation_stations[i]->erase(m_reservation_stations[i]->begin() + j);
						j--;
						done_removing = true;
						break;
					}
				}
				if (done_removing)
				{
					break;
				}
			}
			
			
			if (entry_to_remove->m_state != EMPTY)
			{
				if ((entry_to_remove->m_destination_register != NULL) &&
					(entry_to_remove->m_destination_register->m_rob_entry == entry_to_remove))
				{
					//assert(false);
					
					Register* dest_register = entry_to_remove->m_destination_register;
					entry_to_remove->m_destination_register = NULL;
					dest_register->m_rob_entry = NULL;
					
					ROB_Entry* des_entry = m_head;
					for (int i = 0; i != ROB_SIZE; des_entry = des_entry->m_next, i++)
					{
						if (des_entry->m_state == EMPTY)
						{
							break;
						}
						else if (des_entry->m_destination_register == dest_register)
						{
							dest_register->m_rob_entry = des_entry;
						}
					}
					
				}
				entry_to_remove->m_state = EMPTY;
				entry_to_remove->m_busy = false;
				m_num_of_issues_per_cycle.back()--;
				//std::cout<<"REMOVING: "<<entry_to_remove->m_instruction->m_raw_instruction<<std::endl;
				rob_slot_counter--;
			}
			
			
			entry_to_remove = entry_to_remove->m_next;
		}
		//std::cout<<"FLUSHED"<<std::endl;
		//assert(false);
	}
	
	if (m_head->m_state == EMPTY)
	{
		return false;
	}
	
	
	ROB_Entry* temp = m_head;
	while (temp->m_entry_number != 0)
	{
		temp = temp->m_next;
	}
	
	std::cout<<"R1: "<<m_int_register[1].m_data<<std::endl;
	std::cout<<"R3: "<<m_int_register[3].m_data<<std::endl;
	std::cout<<"R5: "<<m_int_register[5].m_data<<std::endl<<std::endl;
	for (int i = 0; i != ROB_SIZE; temp = temp->m_next, i++)
	{
		std::cout<<temp->m_entry_number<<"\t";
		switch(temp->m_state)
		{
			case EMPTY:
				std::cout<<"EMPTY"<<"\t";
				break;
			case ISSUE:
				std::cout<<"ISSUE"<<"\t";
				break;
			case EXECUTING:
				std::cout<<"EXECUTING"<<"\t";
				break;
			case EXECUTION_COMPLETE:
				std::cout<<"EXECUTION COMPLETE"<<"\t";
				break;
			case WRITING:
				std::cout<<"WRITING"<<"\t";
				break;
			case COMMIT:
				std::cout<<"COMMIT"<<"\t";
				break;
			case WAITING:
				std::cout<<"WAITING"<<"\t";
				break;
			//case BRANCH:
			//	std::cout<<"BRANCH"<<"\t";
			//	break;
		}
		if (temp->m_state != EMPTY)
		{
			std::cout<<temp->m_instruction->m_raw_instruction;
			//if ((temp->m_instruction->m_instruction == SLTI) ||
			//	(temp->m_instruction->m_instruction == SUBI))
			//{
			//	std::cout<<"\t"<<temp->m_value;
			//}
			std::cout<<std::endl;
		}
		else
		{
			std::cout<<std::endl;
		}
	}
	
	//std::cout<<"Reservation Station"<<std::endl;
	//for (int j=0; j < load_reservation_stations.size(); j++)
	//{
	//	std::cout<<load_reservation_stations[j].m_rob_entry->m_instruction->m_raw_instruction<<" ";
	//	std::cout<<load_reservation_stations[j].m_rob_entry->m_state<<std::endl;
	//}
	//std::cout<<std::endl;
	
	m_cycle_number++;
	
	assert(m_num_of_issues_per_cycle.size() == m_num_of_writes_per_cycle.size());
	assert(m_num_of_writes_per_cycle.size() == m_num_of_commits_per_cycle.size());
	return true;
}

// Initialize the ROB Entry numbers
unsigned int ROB_Entry::s_next_id = 0;

