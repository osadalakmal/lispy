CXX:=g++
CC:=gcc
LIBRARY_PATH=$(shell echo "/usr/lib/$(gcc -print-multiarch)")
C_INCLUDE_PATH=$(shell echo "/usr/include/$(gcc -print-multiarch)")
CPLUS_INCLUDE_PATH=$(shell echo "/usr/include/$(gcc -print-multiarch)")
LDFLAGS := -lreadline
LD = g++
CFLAGS = -g -Wall
CPPFLAGS = -g -Wall
RM = /bin/rm -f
SRCS = $(wildcard src/*.cpp) $(wildcard src/*.c)  
OBJS = $(patsubst %.cpp,%.o,$(patsubst %.c,%.o ,$(SRCS)))
PROG = lispy.tsk

all: $(PROG)

$(PROG): $(OBJS)
	$(LD) $(OBJS) -o $(PROG) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@
	
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(PROG) $(OBJS)

depend:
	$(RM) .depend
	@echo $(LIBRARY_PATH) $(C_INCLUDE_PATH) $(CPLUS_INCLUDE_PATH)
	makedepend -f- -- $(CFLAGS) -- $(SRCS) > .depend
        
include .depend