CXX = g++
CXXFLAGS = -O2 -std=c++17
OPENMP = -fopenmp

OBJS = Punto.cc VectorPuntos.cc

all: medios_serial medios_omp

medios_serial: medios.cc $(OBJS)
	$(CXX) $(CXXFLAGS) medios.cc $(OBJS) -o medios_serial

medios_omp: medios_omp.cc $(OBJS)
	$(CXX) $(CXXFLAGS) medios_omp.cc $(OBJS) $(OPENMP) -o medios_omp

clean:
	rm -f *.o medios_serial medios_omp ci0117.eps
