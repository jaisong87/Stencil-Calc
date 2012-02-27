# Makefile for Cilk++ example.  See source code for details.

CILKPP	= cilk++
LIBARG	= -O1 -g -lcilkutil
#PAPIINC = -I /home1/01951/jaisong/papi-4.2.0/src/ /home1/01951/jaisong/papi-4.2.0/src/libpapi.a 
PAPIINC = -I$(TACC_PAPI_INC) -L $(TACC_PAPI_LIB) -lpapi
TARGET	= parallelStencil
SRC	= $(addsuffix .cilk,$(TARGET))

all: $(TARGET)

$(TARGET): $(SRC)
	$(CILKPP) $(SRC) $(PAPIINC) $(LIBARG) -o $@

clean:
	rm -f $(TARGET)
