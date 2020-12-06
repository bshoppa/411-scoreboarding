#include <iostream>
#include <map>
#include <utility>
#include <string>
#include <fstream>
#include <vector>
#include <iterator>
using namespace std;

typedef int clockCycle;

enum InstructionType {
	MEM_INSTRUCTION,
	ALU_INSTRUCTION
};

enum datatype {
	INT,
	FLOAT
};

class Scoreboard;
struct CPUOperation {
	void (*operation) (Scoreboard&, string, string, string) ;
};

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

struct AInput {
public:
	AInput() {
		for(int i = 0; i < 4; i++){
			clockCycleTimes[i] = 0;
		}
	};
	AInput(string t_InstructionName, string t_reg1 = "", string t_reg2 = "", string t_reg3 = "") : InstructionName(t_InstructionName), reg1(t_reg1), reg2(t_reg2), reg3(t_reg3){
		for(int i = 0; i < 4; i++){
			clockCycleTimes[i] = 0;
		}
	};
	AInput(const char* t_InstructionName, const char* t_reg1 = nullptr, const char* t_reg2 = nullptr, const char* t_reg3 = nullptr) : AInput((string) t_InstructionName, (string) t_reg1, (string) t_reg2, (string) t_reg3) {

	}
	AInput(const AInput &in){
		InstructionName = in.InstructionName;
		reg1 = in.reg1;
		reg2 = in.reg2;
		reg3 = in.reg3;
		type = in.type;
		for(int i = 0; i < 4; i++){
			clockCycleTimes[i] = in.clockCycleTimes[i];
		}
		hasExecuted = in.hasExecuted;
	};

	AInput& operator=(const AInput &in){
		InstructionName = in.InstructionName;
		reg1 = in.reg1;
		reg2 = in.reg2;
		reg3 = in.reg3;
		type = in.type;
		for(int i = 0; i < 4; i++){
			clockCycleTimes[i] = in.clockCycleTimes[i];
		}
		hasExecuted = in.hasExecuted;
		return *this;
	};

	~AInput(){

	};
	string InstructionName = "";
	string reg1 = "";
	string reg2 = "";
	string reg3 = "";

	InstructionType type;
	clockCycle clockCycleTimes[4] = {0};
	bool hasExecuted = false;

	clockCycle ReadDependency(AInput &in){
		if(reg1 == in.reg1){

		}
		return 0;
	};

	clockCycle WritebackDependency(AInput &in){
		return clockCycleTimes[3];
	};

	clockCycle Issue(AInput &in){
		if(ReadDependency(in)){

		}
		return clockCycleTimes[1];
	}
};

int max(int a, int b){
	if(a > b){
		return a;
	}
	return b;
}

class Data {
	union {
		int i;
		float f;
	} data;
	enum datatype datatype;
};

void find_next(const string buf, string &out){
	size_t next_comma = buf.find_first_of(',');
	size_t next_space = buf.find_first_of(' ') + 1;

	out = buf.substr(next_space, next_comma - next_space);
};

void find_next(const string buf, string &out, string &nextbuf){
	cout << buf << endl;
	size_t next_comma = buf.find_first_of(',');
	size_t next_space = buf.find_first_of(' ') + 1;

	out = buf.substr(next_space, next_comma - next_space);
	nextbuf = buf.substr(next_comma + 2, string::npos);
	cout << "Next is " << nextbuf << endl;
};

class Scoreboard {
public:
	class Executer {
		// knows what part of the CPU the instruction uses to execute, and knows what function to call to modify the data
	public:
		Subprocessor* part_of_cpu;
		CPUOperation operation;
	};
	map<string, Executer> InstructionUseStatus;
	vector<Subprocessor> Processor;
	vector<AInput> Instructions;
	vector<Data> Memory = vector<Data>(19, Data());
	vector<Data> Registers = vector<Data>(32, Data());
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
		}

		delete[] arr;
	};

	//once each line is parsed, goes into instruction vector
	void load_instructions(string file){
		fstream dataFile;
		string buf;
		AInput temp;
		temp.hasExecuted = false;

		dataFile.open(file);

		while(getline(dataFile, buf, '\n')){

			temp.InstructionName = buf.substr(0, buf.find_first_of(' '));
			string reg1_and_buf; find_next(buf, temp.reg1, reg1_and_buf);
			string reg2_and_buf; find_next(reg1_and_buf, temp.reg2, reg2_and_buf);
			string reg3_and_buf; find_next(reg2_and_buf, temp.reg3);

			printf("Name \"%s\" reg1 \"%s\" reg2 \"%s\" reg3 \"%s\"\n", temp.InstructionName.c_str(), temp.reg1.c_str(), temp.reg2.c_str(), temp.reg3.c_str());

			Instructions.push_back(temp);
		}
	}

	void tryToExecute(AInput &instruction, vector<AInput>::iterator iterator){
		clockCycle whenCanStartIssue = 1;
		clockCycle whenCanWriteBack = 1;
		for(int index = distance(Instructions.begin(), iterator) - 1; index >= 0; index--){
			// check all previous instructions
			auto previousInstruction = Instructions[index];

			if(previousInstruction.hasExecuted){
				whenCanStartIssue = max(previousInstruction.Issue(instruction), whenCanStartIssue);
			}
		}
		instruction.clockCycleTimes[0] = whenCanStartIssue;
		// first = issue
		instruction.clockCycleTimes[1] = instruction.clockCycleTimes[0] + 1;
		// second = read operand
		if(InstructionUseStatus.count(instruction.InstructionName) != (size_t) NULL){
			Subprocessor* processor = InstructionUseStatus.at(instruction.InstructionName).part_of_cpu;
			instruction.clockCycleTimes[2] = instruction.clockCycleTimes[1] + processor->clockCycles[2];
		} else {
			cout << "Not setting clockCycleTimes[2] until mapping is implemented" << endl;
			instruction.clockCycleTimes[2] = instruction.clockCycleTimes[1] + 1;
		}
		// the third is the execute
		whenCanWriteBack = instruction.clockCycleTimes[2] + 1;
		bool changed = true;
		while(changed){
			changed = false;
			for(int index = distance(Instructions.begin(), iterator) - 1; index >= 0; index--){
				// check all previous instructions
				auto previousInstruction = Instructions[index];

				if(previousInstruction.hasExecuted){
					if(previousInstruction.WritebackDependency(instruction) == whenCanWriteBack){
						changed = true;
						whenCanWriteBack++;
						break;
					}
				}
			}
		}

		instruction.clockCycleTimes[3] = whenCanWriteBack;
		//
		instruction.hasExecuted = true;
	}

	void process() {


		// start at instruction[0];
		complete = false;
		int currentClockCycle = 0;
		for(vector<AInput>::iterator CurrentInstruction = Instructions.begin(); CurrentInstruction != Instructions.end(); CurrentInstruction++){
			tryToExecute(*CurrentInstruction, CurrentInstruction);
		}
	}

};

void load(Scoreboard &scoreboard, string arg1, string arg2, string arg3) {

};
void store(Scoreboard &scoreboard, string arg1, string arg2, string arg3) {

};
void fp_add(Scoreboard &scoreboard, string arg1, string arg2, string arg3) {

};
void fp_subtract(Scoreboard &scoreboard, string arg1, string arg2, string arg3) {

};
void fp_multiply(Scoreboard &scoreboard, string arg1, string arg2, string arg3) {

};
void fp_divide(Scoreboard &scoreboard, string arg1, string arg2, string arg3) {

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
			pair<string, pair<string, CPUOperation>>("S.D", pair<string, CPUOperation> ("INTEGER UNIT", CPUOperation{&store})),
			pair<string, pair<string, CPUOperation>>("ADDI", pair<string, CPUOperation> ("INTEGER UNIT", CPUOperation{&fp_add})),
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

	if(argc <= 1){
		myInput.Instructions.push_back(AInput ("L.D", "F2", "0($1)"));
		myInput.Instructions.push_back(AInput ("MUL.D", "F4", "F2", "F0"));
		myInput.Instructions.push_back(AInput ("L.D", "F6", "0($2)"));
		myInput.Instructions.push_back(AInput ("ADD.D", "F6", "F4", "F2"));
		myInput.Instructions.push_back(AInput ("S.D", "F6", "0", "F2"));
		myInput.Instructions.push_back(AInput ("SUB.D", "F3", "F6", "F0"));
		myInput.Instructions.push_back(AInput ("S.D", "F3", "0", "F3"));
	} else {
		//myInput.Instructions
		cout << "File is " << string(argv[1]) << endl;
		myInput.load_instructions(string(argv[1]));
	}
	// insert memory and instructions before executing
	myInput.process();
	printf("%-7s%-6s%-6s%-6s%-10s%-10s%-10s%-10s\n", "Name", "op1", "op2", "op3", "Issue", "Read Oper", "Execute", "Writeback");
	for(size_t i = 0; i < myInput.Instructions.size(); i++){
		auto Instruction = myInput.Instructions[i];
		printf("%-7s%-6s%-6s%-6s%-10i%-10i%-10i%-10i\n", Instruction.InstructionName.c_str(), Instruction.reg1.c_str(), Instruction.reg2.c_str(), Instruction.reg3.c_str(), Instruction.clockCycleTimes[0], Instruction.clockCycleTimes[1], Instruction.clockCycleTimes[2], Instruction.clockCycleTimes[3]);
	}
	return 0;
}
