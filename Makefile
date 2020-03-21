CXX = g++
EXE = SIM
all:
	$(CXX) $(EXE).cpp -o $(EXE)

clean:
	rm $(EXE)
