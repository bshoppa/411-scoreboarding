#include <iostream>
#include <map>
#include <utility>
#include <string>
#include <fstream>
using namespace std;

typedef clockCycle int;

enum InstructionType {
	MEM_INSTRUCTION,
	ALU_INSTRUCTION
};

class Subprocessor {
public:
	Subprocessor(clockCycle[4] t_clockCycles, int t_units) {
		clockCycles = t_clockCycles;
		units = t_units;
	}
	clockCycle[4] clockCycles;
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
	clockCycle[4] clockCycleTimes = {0};
	bool hasExecuted;

	clockCycle ReadDependency(AInput &in){
		if(reg1 == in.reg1){

		}
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
	enum datatype;
};

class Scoreboard {
public:
	map<string, Subprocessor*> InstructionUseStatus;
	vector<Subprocessor> Processor;
	vector<AInput> Instructions;
	vector<Data> Memory = vector<Data>(Data(), 19);
	vector<Data> Registers = vector<Data>(Data(), 32);
	bool complete;

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

	void tryToExecute(){
		AInput instruction = &iterator; // should already be given to you by reference
		clockCycle whenCanStartIssue = 1;
		for(vector<AInput>::iterator it = iterator; it != Instructions.begin(); it--){
			if(it.hasExecuted){
				clockCycle whenCanStartIssue = max(Issue, whenCanStartIssue);
			}
		}
		instruction.clockCycleTimes[0] = whenCanStartIssue;
		// first = issue
		instruction.clockCycleTimes[1] = clockCycleTimes[0] + 1;
		// second = read operand
		instruction.clockCycleTimes[2] = clockCycleTimes[1] + 1;
		// the third is the execute
		instruction.clockCycleTimes[3] = clockCycleTimes[2] + InstructionUseStatus[instruction.InstructionName];
		instruction.hasExecuted = true;
	}

	void process() {
		vector<AInput>::iterator CurrentInstruction = Instructions.begin();
		for(CurrentInstruction)

		// start at instruction[0];
		complete = false;
		int currentClockCycle = 0;
		while(!complete){
			tryToExecute(CurrentInstruction);
			complete = CurrentInstruction == Instructions.end();
		}
	}

};

void load(Scoreboard &scoreboard, string arg1, string arg2);
void store(Scoreboard &scoreboard, string arg1, string arg2);
void fp_add(Scoreboard &scoreboard, string arg1, string arg2);
void fp_multiply(Scoreboard &scoreboard, string arg1, string arg2);
void fp_divide(Scoreboard &scoreboard, string arg1, string arg2);

int main(int argc, char* argv[]) {
	Scoreboard myInput (
		vector<pair<string, Subprocessor>> {
			pair<string, Subprocessor>("INTEGER UNIT", Subprocessor {{1, 1, 1, 1}, 1}),
			pair<string, Subprocessor>("FP ADDER", Subprocessor {{1, 1, 2, 1}, 1}),
			pair<string, Subprocessor>("FP MULTIPLIER", Subprocessor {{1, 1, 10, 1}, 1}),
			pair<string, Subprocessor>("FP DIVIDER", Subprocessor {{1, 1, 40, 1}, 2})
		},
		vector<pair<string, pair<string, void*(Scoreboard, string, string)>>>{
			pair<string, pair<string, void(Scoreboard, string, string)>>("L.D", pair<string, void(Scoreboard, string, string)> ("INTEGER UNIT", &load)),
			{"S.D", "INTEGER UNIT", &store},
		}
	);

	myInput.InstructionUseStatus = {
		{"L.D", {.clockCycles = {1, 1, 1, 1}}},
		{"S.D", {.clockCycles = {1, 1, 1, 1}}},
		{"ADD", {.clockCycles = {1, 1, 2, 1}}},
		{"ADDI", {.clockCycles = {1, 1, 10, 1}, .units = 2}},
		{"", {.clockCycles = {1, 1, 40, 1}}},
	};

	if(argc <= 1){
		myInput.instructions.push_back(AInput("L.D", "F2", "0($1)"));
		myInput.instructions.push_back(AInput("MUL.D", "F2", "0($1)"));
		// todo: continue adding instructions from the assignment here
	} else {
		myInput.instructions
	}
	// insert memory and instructions before executing
	myInput.process();
	return 0;
}
