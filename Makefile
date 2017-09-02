all:
	g++ -std=c++14 -g -lncurses main.cpp -o flames

st:
	g++ -std=c++14 -g -lncurses main.cpp -DSTEP -o flames
