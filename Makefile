CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
INCLUDES = -I../shared -I.

.PHONY: all clean

all: SeuLex.exe SeuYacc.exe

# ====== SeuLex ======
SEULEX_OBJS = seulex/SeuLex.o seulex/LexParser.o seulex/REProcessor.o \
              seulex/NFABuilder.o seulex/DFABuilder.o seulex/DFAMinimizer.o \
              seulex/LexCodeGen.o

SeuLex.exe: $(SEULEX_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

seulex/%.o: seulex/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# ====== SeuYacc ======
SEUYACC_OBJS = seuyacc/SeuYacc.o seuyacc/GrammarParser.o seuyacc/DFAGenerator.o \
               seuyacc/Emitter.o

SeuYacc.exe: $(SEUYACC_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

seuyacc/%.o: seuyacc/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f seulex/*.o seuyacc/*.o SeuLex.exe SeuYacc.exe
