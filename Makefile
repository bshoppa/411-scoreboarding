scoreboard: scoreboard.cpp
	g++ scoreboard.cpp -g -o scoreboard
val: scoreboard
	valgrind ./scoreboard instructions.txt
