# This file has has two main targets. Code which can be built portably, and
# code which will really only run on the pi.

ifndef CXX
  CXX=g++
endif

COMMONFLAGS=-Wall -std=c++17
ifdef NONRPI
  CXXFLAGS=$(COMMONFLAGS) -g
else
  CXXFLAGS=$(COMMONFLAGS) -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host -I/opt/vc/include/interface/vmcs_host/linux -L/opt/vc/lib
endif

ifdef NONRPI
  LIBS = -lfftw3 -lm -lmosquitto -lmosquittopp -lpthread
else
  LIBS = -lwiringPi -lmosquitto -lmosquittopp -lrt -lfftw3 -lm -lpthread -Wl,-rpath=/opt/vc/lib
endif

COMMON_SOURCES=circular_queue.cpp fft.cpp spectrum_analyzer.cpp publisher.cpp
ifdef NONRPI
  SOURCES=$(COMMON_SOURCES) offline_main.cpp 
else
  SOURCES=$(COMMON_SOURCES) main.cpp hardware.cpp
endif

BUILD_DIR=build

ifndef INSTALL_LOCATION 
  INSTALL_LOCATION=/usr/local/bin
endif

OBJS=$(SOURCES:%.cpp=$(BUILD_DIR)/%.o)

EXE=sense
EXE_FILE=$(BUILD_DIR)/$(EXE)

all: $(EXE_FILE)

$(EXE_FILE): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LIBS) $(CXXFLAGS)

build:
	mkdir build

build/%.o: %.cpp build
	$(CXX) -c -o $@ $< $(CXXFLAGS)

install: $(EXE_FILE)
	cp $(EXE_FILE) $(INSTALL_LOCATION)/$(EXE)

clean: build
	rm -rf build
