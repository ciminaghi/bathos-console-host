SDK ?= ../SaleaeDeviceSdk-1.1.14

CC=gcc
CXX=g++
LIBSALEAE = -lSaleaeDevice64
CXXFLAGS = -I$(SDK)/include -O3 -w
LDFLAGS = -L$(SDK)/lib $(LIBSALEAE)

EXE = bathos_console

all: $(EXE)

clean:
	rm -f $(EXE) *.o $~
