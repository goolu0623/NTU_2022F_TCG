all:
	g++ -std=c++11 -D GREEDY src/baseline.cpp -o greedy
	g++ -std=c++11 -D RANDOM src/baseline.cpp -o random
	g++ -std=c++11 -D HEURISTIC src/baseline.cpp -o heuristic
	g++ -std=c++11 -D MCTS src/baseline.cpp -o pure_mcts

random:
	g++ -std=c++11 -D RANDOM src/baseline.cpp -o random

greedy:
	g++ -std=c++11 -D GREEDY src/baseline.cpp -o greedy

heuristic:
	g++ -std=c++11 -D HEURISTIC src/baseline.cpp -o heuristic
mcts:
	g++ -std=c++11 -D MCTS src/baseline.cpp -o pure_mcts
clean:
	rm -rf greedy
	rm -rf random
	rm -rf heuristic
	rm -rf pure_mcts
	rm -rf .log.*
