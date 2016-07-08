LIBS=-lsfml-window -lsfml-system -lsfml-graphics -lsfml-audio
CXX=g++
CXFLAGS=-c -Wall -std=c++11
LDFLAGS=$(LIBS)
SOURCES=Main.cpp chip8.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=emuchip8

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CXX) $(CXFLAGS) $< -o $@

.PHONE: clean
clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)
