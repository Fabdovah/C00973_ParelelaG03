# Makefile
CXX = g++
MPICXX = mpicxx
CXXFLAGS = -O2 -std=c++17

all: serial mpi

serial: lcs_serial
mpi: lcs_mpi

lcs_serial: lcs_serial.cc adn.cc adn.h
	$(CXX) $(CXXFLAGS) lcs_serial.cc adn.cc -o lcs_serial

lcs_mpi: lcs_mpi.cc adn.cc adn.h
	$(MPICXX) $(CXXFLAGS) lcs_mpi.cc adn.cc -o lcs_mpi

clean:
	rm -f lcs_serial lcs_mpi
