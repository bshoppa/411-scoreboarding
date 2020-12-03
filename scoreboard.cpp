#include <iostream>
#include <map>
#include <utility>
#include <string>
#include <fstream>
using namespace std;

enum InstructionType {
	MEM_INSTRUCTION,
	ALU_INSTRUCTION
};

class Subprocessor {
public:
	int[4] clockCycles;
	int units = 1;
};

class AInput {
	string InstructionName;
	string reg1;
	string reg2;
	string reg3;

	InstructionType type;
	int[4] clockCycleTimes = {0};
	bool hasExecuted;
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

			Instructions.psuh_back(temp);
		}
	}

	void tryToExecute(){
		AInput instruction = &iterator; // should already be given to you by reference
		int start = 0;
		for(vector<AInput>::iterator it = iterator; it != Instructions.begin(); it--){
			if(it.hasExecuted){
				start = max(&it.clockCycleTimes[3], start);
			}
		}
		instruction.clockCycleTimes[0] = start + 1;
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

int main(int argc, char* argv[]) {
	Scoreboard myInput {
		.Status = {
			{"L.D", {.clockCycles = {1, 1, 1, 1}}},
			{"S.D", {.clockCycles = {1, 1, 1, 1}}},
			{"ADD", {.clockCycles = {1, 1, 2, 1}}},
			{"ADDI", {.clockCycles = {1, 1, 10, 1}, .units = 2}},
			{"", {.clockCycles = {1, 1, 40, 1}}},
		},

	};
	if(argc <= 1){
		myInput.instructions.push_back(AInput());
	} else {
		myInput.status
	}
	return 0;
}
