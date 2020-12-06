// Bailey Shoppa & Matthew Davis
// 411 Scoreboarding



#include <iostream>
#include <map>
#include <utility>
#include <string>
#include <fstream>
#include <vector>
#include <iterator>
#include <exception>
using namespace std;

// clockCycles are used as the time values (in terms of clock cycles) in the scoreboard.
// an array of 4 clockCycles represents values associated with Issue, Read, Execute, and Writeback
typedef int clockCycle;

enum datatype {
	INT,
	FLOAT
};

class Scoreboard;

// attached to instructions - can consist of an operation like "add", "subtract", etc.
// accesses the data members of the input scoreboard
struct CPUOperation {
	void (*operation) (Scoreboard&, string, string, string) ;
	bool doesWriteback = false;
};

// determines specifications of a part of the CPU - e.g. the INTEGER UNIT, a certain floating point unit such as the divider, etc.
// also determines how long it takes to issue, read oper, execute, and writeback
// (however, issue, read oper, and writeback don't technically occupy a particular subprocessor)
class Subprocessor {
public:
	Subprocessor(const clockCycle t_clockCycles[4], int t_units) {
		for(int i = 0; i < 4; i++){
			clockCycles[i] = t_clockCycles[i];
		}
		units = t_units;
	}
	clockCycle clockCycles[4];
	int units = 1;
	string name;
};

// contains the text values of each input MIPS-like line, as well as scoreboarding data and scoreboarding functions
// examples of how a string is parsed and placed into an Instruction.
// of course, the Instruction is 4 strings
// INSTRUCTION reg1, reg2, reg3
// -> .InstructionName, .reg1, .reg2, .reg3
// INSTRUCTION reg1, offset (address)
// -> .InstructionName, .reg1, .reg2 ... .reg3 = ""
// INSTRUCTION reg1, reg2, IMMEDIATE
// -> .InstructionName, .reg1, .reg2, .reg3
struct Instruction {
public:
	string InstructionName = "";
	string reg1 = "";
	string reg2 = "";
	string reg3 = "";

	clockCycle clockCycleTimes[4] = {0};
	bool hasExecuted = false;

	bool doesWriteback = false;

	Instruction() {
		for(int i = 0; i < 4; i++){
			clockCycleTimes[i] = 0;
		}
	};
	Instruction(string t_InstructionName, string t_reg1 = "", string t_reg2 = "", string t_reg3 = "") : InstructionName(t_InstructionName), reg1(t_reg1), reg2(t_reg2), reg3(t_reg3){
		for(int i = 0; i < 4; i++){
			clockCycleTimes[i] = 0;
		}
	};
	Instruction(const char* t_InstructionName, const char* t_reg1 = nullptr, const char* t_reg2 = nullptr, const char* t_reg3 = nullptr) : Instruction((string) t_InstructionName, (string) t_reg1, (string) t_reg2, (string) t_reg3) {

	}

	clockCycle IssueDependency(Instruction &in){
		if(doesWriteback){
			return clockCycleTimes[1];
		}
		if(in.reg1 == reg1){
			if(in.doesWriteback){
				return clockCycleTimes[0] + 1;
			}
			return clockCycleTimes[3] + 1;
		}
		return clockCycleTimes[0] + 1; // don't allow anything to issue at the same time
		// and ... force future instructions to execute in the future.
	};

	clockCycle ReadOperDependency(Instruction &in){
		if(doesWriteback){
			return clockCycleTimes[1];
		}
		if(reg1 == in.reg1){
			return clockCycleTimes[3];
		}
		if(reg1 == in.reg2){
			return clockCycleTimes[3];
		}
		if(reg1 == in.reg3){
			return clockCycleTimes[3];
		}
		return 0;
	};

	clockCycle WritebackDependency(Instruction &in){
		return clockCycleTimes[3];
	};

	clockCycle WritebackReadAfterWrite(Instruction &in){
		if(reg2 == in.reg1){
			return clockCycleTimes[1];
		}
		if(reg3 == in.reg1){
			return clockCycleTimes[1];
		}
		return 0;
	}

};

int max(int a, int b){
	if(a > b){
		return a;
	}
	return b;
}

// Data is for a Register or Memory object to hold its integer / float value.
// After reading the project write up, I would decide that Memory can only hold floating point number, FRegisters can only hold floating point numbers, and IRegisters can only hold integers.
// This is not "hardcoded", it is just that there are no functions you can call to make Memory store a floating point number.
class Data {
public:
	Data() {

	}
	Data(int v){
		data.i = (int) v;
		datatype = INT;
	};
	Data(float v){
		data.f = (float) v;
		datatype = FLOAT;
	};
	union {
		int i;
		float f;
	} data;
	enum datatype datatype;
};

// parse each line of input
void find_next(const string buf, string &out){
	size_t next_comma = buf.find_first_of(',');

	out = buf.substr(0, next_comma);
};

void find_next(const string buf, string &out, string &nextbuf){
	size_t next_comma = buf.find_first_of(',');

	out = buf.substr(0, next_comma);
	if(next_comma == string::npos){

	} else {
		nextbuf = buf.substr(next_comma + 2, string::npos);
	}
};

// simulate the CPU and memory
// constructor:
// build the simulation so that it can interpret a set of commands and associate them with a specific (simulated) CPU component

// process:
// "advance" the state of the Scoreboard to the end, determine the right clock cycles, and update Memory, FRegisters, and IRegisters
class Scoreboard {
public:
	class Executer {
		// knows what part of the CPU the instruction uses to execute, and knows what function to call to modify the data
		// basically i just got sick of using the pair<> constructor
	public:
		Subprocessor* part_of_cpu;
		CPUOperation operation;
	};
	map<string, Executer> InstructionUseStatus;
	vector<Subprocessor> Processor;
	vector<Instruction> Instructions;
	vector<Data> Memory = vector<Data>(19, Data((float) 0));
	vector<Data> FRegisters = vector<Data>(32, Data((float) 0.));
	vector<Data> IRegisters = vector<Data>(32, Data((int) 0));
	bool complete;

	Scoreboard (vector<pair<string, Subprocessor>> subprocessors, vector<pair<string, pair<string, CPUOperation>>> instruction_definitions) {
		string* arr = new string[subprocessors.size()];
		for(vector<pair<string, Subprocessor>>::iterator it = subprocessors.begin(); it != subprocessors.end(); it++){
			Processor.push_back(it->second);
			arr[distance(subprocessors.begin(), it)] = it->first;
		}

		for(vector<pair<string, pair<string, CPUOperation>>>::iterator it = instruction_definitions.begin(); it != instruction_definitions.end(); it++){
			for(size_t i = 0; i < Processor.size(); i++){
				if(arr[i] == it->second.first){
					InstructionUseStatus[it->first] = Executer {&Processor[i], it->second.second};
					break;
				}
			}
			if(InstructionUseStatus.count(it->first) == (size_t) NULL){
				cout << "WARNING: INSTRUCTION " << it->first << " NOT IN LIST OF SUPPORTED INSTRUCTIONS" << endl;
			}
		}

		delete[] arr;
	};

	//once each line is parsed, goes into instruction vector
	void load_instructions(string file){
		fstream dataFile;
		string buf;
		Instruction temp;
		temp.hasExecuted = false;

		dataFile.open(file);

		while(getline(dataFile, buf, '\n')){

			temp.InstructionName = buf.substr(0, buf.find_first_of(' '));
			string reg1_and_buf; find_next(buf.substr(buf.find_first_of(' ') + 1, string::npos), temp.reg1, reg1_and_buf);
			string reg2_and_buf; find_next(reg1_and_buf, temp.reg2, reg2_and_buf);
			string reg3_and_buf; find_next(reg2_and_buf, temp.reg3);

			//printf("Name \"%s\" reg1 \"%s\" reg2 \"%s\" reg3 \"%s\"\n", temp.InstructionName.c_str(), temp.reg1.c_str(), temp.reg2.c_str(), temp.reg3.c_str());
			if(InstructionUseStatus.count(temp.InstructionName) != (size_t) NULL){
				temp.doesWriteback = InstructionUseStatus.at(temp.InstructionName).operation.doesWriteback;
			}
			Instructions.push_back(temp);
		}
	};

	void load_memory(vector<Data> mem){
		Memory = mem;
	};

	void tryToExecute(Instruction &instruction, vector<Instruction>::iterator iterator){
		clockCycle whenCanStartIssue = 1;
		clockCycle whenCanWriteBack;
		clockCycle whenCanStartReadOper;
		// findIssue: // don't crucify me, i like goto's sometimes ...

		bool hasEnoughUnits = false;
		while(!hasEnoughUnits){
			for(int index = distance(Instructions.begin(), iterator) - 1; index >= 0; index--){
				// check all previous instructions
				// (there is probably a method that doesn't require checking "all" previous instructions)
				auto previousInstruction = Instructions[index];

				if(previousInstruction.hasExecuted){
					whenCanStartIssue = max(previousInstruction.IssueDependency(instruction), whenCanStartIssue);
				}
			}
			int unitsUsedAtTime = 1; // this instruction will use a unit.
			if(InstructionUseStatus.count(instruction.InstructionName) != (size_t) NULL){
				Subprocessor* processor = InstructionUseStatus.at(instruction.InstructionName).part_of_cpu;
				for(int index = distance(Instructions.begin(), iterator) - 1; index >= 0; index--){
					auto previousInstruction = Instructions[index];
					// if both instructions use the same subprocessor, they are using the same unit.
					if(InstructionUseStatus.at(previousInstruction.InstructionName).part_of_cpu == processor){
						// check if both are using the same unit at the same time.
						if(previousInstruction.clockCycleTimes[0] <= whenCanStartIssue && previousInstruction.clockCycleTimes[3] >= whenCanStartIssue){
							unitsUsedAtTime++;
						}
					}
				}
				if(processor->units < unitsUsedAtTime){
					whenCanStartIssue++;
					hasEnoughUnits = false;
				} else {
					hasEnoughUnits = true;
				}
			} else {
				cout << "Unable to find processor unit for instruction." << endl;
			}
		}


		instruction.clockCycleTimes[0] = whenCanStartIssue;
		// first = issue

		whenCanWriteBack = whenCanStartIssue + 1;
		whenCanStartReadOper = whenCanStartIssue + 1;
		// FIND THE OPERATIONS THAT SET THIS ARGUMENT'S REGISTERS
		for(int index = distance(Instructions.begin(), iterator) - 1; index >= 0; index--){
			auto previousInstruction = Instructions[index];

			if(previousInstruction.hasExecuted){
				whenCanStartReadOper = max(previousInstruction.ReadOperDependency(instruction) + 1, whenCanStartReadOper);
			}
		}
		instruction.clockCycleTimes[1] = whenCanStartReadOper;
		// second = read operand

		// try to find out information about the type of the instruction inside of the Scoreboard object.
		if(InstructionUseStatus.count(instruction.InstructionName) != (size_t) NULL){
			Subprocessor* processor = InstructionUseStatus.at(instruction.InstructionName).part_of_cpu;
			// processor indicates how long an operation takes (clockCycles)
			// and how many of operations of that type can happen simultaneously
			instruction.clockCycleTimes[2] = instruction.clockCycleTimes[1] + processor->clockCycles[2];

			Scoreboard thisScoreboard = *this;
			(InstructionUseStatus.at(instruction.InstructionName).operation.operation)(thisScoreboard, instruction.reg1, instruction.reg2, instruction.reg3);
			*this = thisScoreboard;
		} else {
			cout << "Not setting clockCycleTimes[2] until mapping is implemented" << endl;
			instruction.clockCycleTimes[2] = instruction.clockCycleTimes[1] + 1;
		}
		// third = execute
		whenCanWriteBack = instruction.clockCycleTimes[2] + 1;
		bool changed = true;
		while(changed){
			changed = false;
			for(int index = distance(Instructions.begin(), iterator) - 1; index >= 0; index--){
				// check all previous instructions so that writeback does not occur at the same time as another writeback
				auto previousInstruction = Instructions[index];

				if(previousInstruction.hasExecuted){
					if(previousInstruction.WritebackDependency(instruction) == whenCanWriteBack || previousInstruction.WritebackReadAfterWrite(instruction) >= whenCanWriteBack){
						changed = true;
						// since we cannot go into the past to writeback... we shall go into the future.
						whenCanWriteBack++;
						break;
					}
				}
			}
		}

		instruction.clockCycleTimes[3] = whenCanWriteBack;
		//
		instruction.hasExecuted = true;
	};

	void process() {
		complete = false;
		int currentClockCycle = 0;
		for(vector<Instruction>::iterator CurrentInstruction = Instructions.begin(); CurrentInstruction != Instructions.end(); CurrentInstruction++){
			tryToExecute(*CurrentInstruction, CurrentInstruction);
		}
	}

};







// helper functions for CPU operations
size_t parse_register(string arg){
	return stoi(arg.substr(1));
}
size_t parse_address(string arg){
	size_t start_of_address = arg.find_first_of('(');
	size_t end_of_address = arg.find_first_of(')');
	if(start_of_address != string::npos){
		return stoi(arg.substr(start_of_address + 1, (end_of_address - start_of_address) - 1));
	}
	throw invalid_argument("unable to parse offset and address of load instruction\nuse 'L.D F(register), offset(address)");
	//return 0;
}
size_t parse_offset(string arg){
	size_t start_of_address = arg.find_first_of('(');
	return stoi(arg.substr(0, start_of_address));
}

void captureAddresses(size_t &address_in_F_1, size_t &address_in_F_2, size_t &address_in_F_3, const string arg1, const string arg2, const string arg3, const size_t address_max){
	address_in_F_1 = parse_register(arg1);
	address_in_F_2 = parse_register(arg2);
	address_in_F_3 = parse_register(arg3);

	if(address_in_F_1 >= address_max){
		throw out_of_range("position out of bounds");
	}
	if(address_in_F_1 < 0){
		throw out_of_range("position out of bounds");
	}
	if(address_in_F_2 >= address_max){
		throw out_of_range("position out of bounds");
	}
	if(address_in_F_2 < 0){
		throw out_of_range("position out of bounds");
	}
	if(address_in_F_3 >= address_max){
		throw out_of_range("position out of bounds");
	}
	if(address_in_F_3 < 0){
		throw out_of_range("position out of bounds");
	}
}

// CPU operations
void load(Scoreboard &scoreboard, string arg1, string arg2, string arg3) {
	size_t address_in_F = parse_register(arg1);
	size_t address_in_mem = parse_address(arg2);
	size_t offset_in_mem = parse_offset(arg2);

	if(address_in_mem + offset_in_mem >= scoreboard.Memory.size()){
		throw out_of_range("position in memory out of bounds");
	}
	if(address_in_mem + offset_in_mem < 0){
		throw out_of_range("position in memory out of bounds");
	}
	if(address_in_F >= scoreboard.FRegisters.size()){
		throw out_of_range("position in floating point registers out of bounds");
	}
	if(address_in_F < 0){
		throw out_of_range("position in floating point registers out of bounds");
	}

	scoreboard.FRegisters[address_in_F] = Data((float) scoreboard.Memory[address_in_mem + offset_in_mem].data.i);
};

void store(Scoreboard &scoreboard, string arg1, string arg2, string arg3) {
	size_t address_in_F = parse_register(arg1);
	size_t address_in_mem = parse_address(arg2);
	size_t offset_in_mem = parse_offset(arg2);

	if(address_in_mem + offset_in_mem >= scoreboard.Memory.size()){
		throw out_of_range("position in memory out of bounds");
	}
	if(address_in_mem + offset_in_mem < 0){
		throw out_of_range("position in memory out of bounds");
	}
	if(address_in_F >= scoreboard.FRegisters.size()){
		throw out_of_range("position in floating point registers out of bounds");
	}
	if(address_in_F < 0){
		throw out_of_range("position in floating point registers out of bounds");
	}

	scoreboard.Memory[address_in_mem + offset_in_mem] = Data((int) scoreboard.FRegisters[address_in_F].data.f);
};

void int_add(Scoreboard &scoreboard, string arg1, string arg2, string arg3) {
	size_t address_in_I_1;
	size_t address_in_I_2;
	size_t address_in_I_3;
	try {
		captureAddresses(address_in_I_1, address_in_I_2, address_in_I_3, arg1, arg2, arg3, scoreboard.FRegisters.size());
	}
	catch (const out_of_range&e) {
		throw(out_of_range("position in integer registers out of bounds"));
	}

	scoreboard.IRegisters[address_in_I_1].data.i = scoreboard.IRegisters[address_in_I_2].data.i + scoreboard.IRegisters[address_in_I_3].data.i;
};

void int_add_immediate_value(Scoreboard &scoreboard, string arg1, string arg2, string arg3) {
	size_t address_in_I_1 = parse_register(arg1);
	size_t address_in_I_2 = parse_register(arg1);
	int immediate_value = stoi(arg3);

	scoreboard.IRegisters[address_in_I_1].data.i = scoreboard.IRegisters[address_in_I_2].data.i + immediate_value;
};

void fp_add(Scoreboard &scoreboard, string arg1, string arg2, string arg3) {
	size_t address_in_F_1;
	size_t address_in_F_2;
	size_t address_in_F_3;
	try {
		captureAddresses(address_in_F_1, address_in_F_2, address_in_F_3, arg1, arg2, arg3, scoreboard.FRegisters.size());
	}
	catch (const out_of_range&e) {
		throw(out_of_range("position in fp registers out of bounds"));
	}

	scoreboard.FRegisters[address_in_F_1].data.f = scoreboard.FRegisters[address_in_F_2].data.f + scoreboard.FRegisters[address_in_F_3].data.f;
};

void fp_subtract(Scoreboard &scoreboard, string arg1, string arg2, string arg3) {
	size_t address_in_F_1;
	size_t address_in_F_2;
	size_t address_in_F_3;
	try {
		captureAddresses(address_in_F_1, address_in_F_2, address_in_F_3, arg1, arg2, arg3, scoreboard.FRegisters.size());
	}
	catch (const out_of_range&e) {
		throw(out_of_range("position in fp registers out of bounds"));
	}

	scoreboard.FRegisters[address_in_F_1].data.f = scoreboard.FRegisters[address_in_F_2].data.f - scoreboard.FRegisters[address_in_F_3].data.f;
};

void fp_multiply(Scoreboard &scoreboard, string arg1, string arg2, string arg3) {
	size_t address_in_F_1;
	size_t address_in_F_2;
	size_t address_in_F_3;
	try {
		captureAddresses(address_in_F_1, address_in_F_2, address_in_F_3, arg1, arg2, arg3, scoreboard.FRegisters.size());
	}
	catch (const out_of_range&e) {
		throw(out_of_range("position in fp registers out of bounds"));
	}

	scoreboard.FRegisters[address_in_F_1].data.f = scoreboard.FRegisters[address_in_F_2].data.f * scoreboard.FRegisters[address_in_F_3].data.f;
};

void fp_divide(Scoreboard &scoreboard, string arg1, string arg2, string arg3) {
	size_t address_in_F_1;
	size_t address_in_F_2;
	size_t address_in_F_3;
	try {
		captureAddresses(address_in_F_1, address_in_F_2, address_in_F_3, arg1, arg2, arg3, scoreboard.FRegisters.size());
	}
	catch (const out_of_range&e) {
		throw(out_of_range("position in fp registers out of bounds"));
	}

	scoreboard.FRegisters[address_in_F_1].data.f = scoreboard.FRegisters[address_in_F_2].data.f / scoreboard.FRegisters[address_in_F_3].data.f;
};





int main(int argc, char* argv[]) {
	Scoreboard myInput (
		{
			pair<string, Subprocessor>((string) "INTEGER UNIT", Subprocessor ((const clockCycle[]) {1, 1, 1, 1}, 1)),
			pair<string, Subprocessor>((string) "FP ADDER", Subprocessor ((const clockCycle[]) {1, 1, 2, 1}, 1)),
			pair<string, Subprocessor>((string) "FP MULTIPLIER", Subprocessor ((const clockCycle[]) {1, 1, 10, 1}, 1)),
			pair<string, Subprocessor>((string) "FP DIVIDER", Subprocessor ((const clockCycle[]) {1, 1, 40, 1}, 2))
		},
		{
			pair<string, pair<string, CPUOperation>>("L.D", pair<string, CPUOperation> ("INTEGER UNIT", CPUOperation{&load})),
			pair<string, pair<string, CPUOperation>>("S.D", pair<string, CPUOperation> ("INTEGER UNIT", CPUOperation{&store, true})),
			pair<string, pair<string, CPUOperation>>("ADD", pair<string, CPUOperation> ("INTEGER UNIT", CPUOperation{&int_add})),
			pair<string, pair<string, CPUOperation>>("ADDI", pair<string, CPUOperation> ("INTEGER UNIT", CPUOperation{&int_add_immediate_value})),
			pair<string, pair<string, CPUOperation>>("MUL.D", pair<string, CPUOperation> ("FP MULTIPLIER", CPUOperation{&fp_multiply})),
			pair<string, pair<string, CPUOperation>>("DIV.D", pair<string, CPUOperation> ("FP DIVIDER", CPUOperation{&fp_divide})),
			pair<string, pair<string, CPUOperation>>("ADD.D", pair<string, CPUOperation> ("FP ADDER", CPUOperation{&fp_add})),
			pair<string, pair<string, CPUOperation>>("SUB.D", pair<string, CPUOperation> ("FP ADDER", CPUOperation{&fp_subtract})),
		}
	);

	/*myInput.InstructionUseStatus = {
		{"L.D", {.clockCycles = {1, 1, 1, 1}}},
		{"S.D", {.clockCycles = {1, 1, 1, 1}}},
		{"ADD", {.clockCycles = {1, 1, 2, 1}}},
		{"ADDI", {.clockCycles = {1, 1, 10, 1}, .units = 2}},
		{"", {.clockCycles = {1, 1, 40, 1}}},
	};*/

	{
		const vector<Data> data {
			45,
			12,
			0,
			0,
			10,
			135,
			254,
			127,
			18,
			4,
			55,
			8,
			2,
			98,
			13,
			5,
			233,
			158,
			167,
		};
		myInput.load_memory(data);
	}


	if(argc <= 1){
		myInput.Instructions.push_back(Instruction ("L.D", "F2", "0($1)"));
		myInput.Instructions.push_back(Instruction ("MUL.D", "F4", "F2", "F0"));
		myInput.Instructions.push_back(Instruction ("L.D", "F6", "0($2)"));
		myInput.Instructions.push_back(Instruction ("ADD.D", "F6", "F4", "F2"));
		myInput.Instructions.push_back(Instruction ("S.D", "F6", "0", "F2"));
		myInput.Instructions.push_back(Instruction ("SUB.D", "F3", "F6", "F0"));
		myInput.Instructions.push_back(Instruction ("S.D", "F3", "0", "F3"));
	} else {
		//myInput.Instructions
		myInput.load_instructions(string(argv[1]));
	}
	// insert memory and instructions before executing
	myInput.process();
	printf("%-7s%-6s%-6s%-6s%-10s%-10s%-10s%-10s\n", "Name", "op1", "op2", "op3", "Issue", "Read Oper", "Execute", "Writeback");
	for(size_t i = 0; i < myInput.Instructions.size(); i++){
		auto Instruction = myInput.Instructions[i];
		printf("%-7s%-6s%-6s%-6s%-10i%-10i%-10i%-10i\n", Instruction.InstructionName.c_str(), Instruction.reg1.c_str(), Instruction.reg2.c_str(), Instruction.reg3.c_str(), Instruction.clockCycleTimes[0], Instruction.clockCycleTimes[1], Instruction.clockCycleTimes[2], Instruction.clockCycleTimes[3]);
	}
	printf("F Registers:\n");
	for(size_t i = 0; i < myInput.FRegisters.size(); i++){
		printf("F%i: %f\n", i, myInput.FRegisters[i].data.f);
	}
	printf("I Registers:\n");
	for(size_t i = 0; i < myInput.IRegisters.size(); i++){
		printf("$%i: %i\n", i,  myInput.IRegisters[i].data.i);
	}
	return 0;
}
