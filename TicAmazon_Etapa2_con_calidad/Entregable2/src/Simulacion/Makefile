CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pthread
TARGET := simulación
# formato.cc pertenece al FileSystem anterior y no se usa con el del servidor.
FUENTES := $(filter-out formato.cc,$(wildcard *.cc))

all: $(TARGET)

$(TARGET): $(FUENTES) $(wildcard *.h)
	$(CXX) $(CXXFLAGS) $(FUENTES) -o $@

run: $(TARGET)
	./$(TARGET) $(ARGS)

clean:
	rm -f $(TARGET) bitacora.log

.PHONY: all run clean
