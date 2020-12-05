#include <iostream>
#include <map>
#include <utility>
#include <string>
#include <fstream>
#include <vector>
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

class AInput {
public:
	string InstructionName;
	string reg1;
	string reg2;
	string reg3;

	InstructionType type;
	clockCycle clockCycleTimes[4] = {0};
	bool hasExecuted;

	clockCycle ReadDependency(AInput &in){
		if(reg1 == in.reg1){

		}
		return 0;
	};

	clockCycle Issue(AInput &in){
		if(ReadDependency(in)){

		}
		return clockCycleTimes[2];
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

class Scoreboard {
public:
	map<string, Subprocessor*> InstructionUseStatus;
	vector<Subprocessor> Processor;
	vector<AInput> Instructions;
	vector<Data> Memory = vector<Data>(19, Data());
	vector<Data> Registers = vector<Data>(32, Data());
	bool complete;

	Scoreboard (vector<pair<string, Subprocessor>> subprocessors, vector<pair<string, pair<string, void (*) (Scoreboard&, string, string)>>> instruction_definitions) {

	};

	//once each line is parsed, goes into instruction vector
	void load_instructions(string file){
		fstream dataFile;
		string buf;
		AInput temp;

		dataFile.open(file);

		while(getline(dataFile, buf, '\n')){
			temp.InstructionName = buf;
			temp.reg1 = buf;
			temp.reg2 = buf;
			temp.reg3 = buf;

			Instructions.push_back(temp);
		}
	}

	void tryToExecute(vector<AInput>::iterator iterator){
		AInput instruction = *iterator; // should already be given to you by reference
		clockCycle whenCanStartIssue = 1;
		for(vector<AInput>::iterator it = iterator; it != Instructions.begin(); it--){
			if(it->hasExecuted){
				clockCycle whenCanStartIssue = max(it->Issue(instruction), whenCanStartIssue);
			}
		}
		instruction.clockCycleTimes[0] = whenCanStartIssue;
		// first = issue
		instruction.clockCycleTimes[1] = instruction.clockCycleTimes[0] + 1;
		// second = read operand
		instruction.clockCycleTimes[2] = instruction.clockCycleTimes[1] + 1;
		// the third is the execute
		instruction.clockCycleTimes[3] = instruction.clockCycleTimes[2] + InstructionUseStatus.at(instruction.InstructionName)->clockCycles[2];
		instruction.hasExecuted = true;
	}

	void process() {
		vector<AInput>::iterator CurrentInstruction = Instructions.begin();

		// start at instruction[0];
		complete = false;
		int currentClockCycle = 0;
		while(!complete){
			tryToExecute(CurrentInstruction);
			complete = CurrentInstruction == Instructions.end();
			CurrentInstruction++;
		}
	}

};

void load(Scoreboard &scoreboard, string arg1, string arg2) {

};
void store(Scoreboard &scoreboard, string arg1, string arg2) {

};
void fp_add(Scoreboard &scoreboard, string arg1, string arg2) {

};
void fp_subtract(Scoreboard &scoreboard, string arg1, string arg2) {

};
void fp_multiply(Scoreboard &scoreboard, string arg1, string arg2) {

};
void fp_divide(Scoreboard &scoreboard, string arg1, string arg2) {

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
			pair<string, pair<string, void (*) (Scoreboard&, string, string)>>("L.D", pair<string, void (*) (Scoreboard&, string, string)> ("INTEGER UNIT", &load)),
			pair<string, pair<string, void (*) (Scoreboard&, string, string)>>("S.D", pair<string, void (*) (Scoreboard&, string, string)> ("INTEGER UNIT", &store)),
			pair<string, pair<string, void (*) (Scoreboard&, string, string)>>("MUL.D", pair<string, void (*) (Scoreboard&, string, string)> ("FP MULTIPLIER", &fp_multiply)),
			pair<string, pair<string, void (*) (Scoreboard&, string, string)>>("DIV.D", pair<string, void (*) (Scoreboard&, string, string)> ("FP DIVIDER", &fp_divide)),
			pair<string, pair<string, void (*) (Scoreboard&, string, string)>>("ADD.D", pair<string, void (*) (Scoreboard&, string, string)> ("FP ADDER", &fp_add)),
			pair<string, pair<string, void (*) (Scoreboard&, string, string)>>("SUB.D", pair<string, void (*) (Scoreboard&, string, string)> ("FP ADDER", &fp_subtract)),
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
		myInput.Instructions.push_back(AInput {"L.D", "F2", "0($1)"});
		myInput.Instructions.push_back(AInput {"MUL.D", "F4", "F2", "F0"});
		myInput.Instructions.push_back(AInput {"L.D", "F6", "0($2)"});
		myInput.Instructions.push_back(AInput {"ADD.D", "F6", "F4", "F2"});
		myInput.Instructions.push_back(AInput {"S.D", "F6", "0", "F2"});
		myInput.Instructions.push_back(AInput {"SUB.D", "F3", "F6", "F0"});
		myInput.Instructions.push_back(AInput {"S.D", "F3", "0", "F3"});
	} else {
		//myInput.Instructions
	}
	// insert memory and instructions before executing
	myInput.process();
	return 0;
}
