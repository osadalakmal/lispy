CXX:=g++
CC:=gcc
LIBRARY_PATH=$(shell echo "/usr/lib/$(gcc -print-multiarch)")
C_INCLUDE_PATH=$(shell echo "/usr/include/$(gcc -print-multiarch)")
CPLUS_INCLUDE_PATH=$(shell echo "/usr/include/$(gcc -print-multiarch)")
LDFLAGS := -lreadline
LD = g++
CFLAGS = -g -Wall -I./
CPPFLAGS = -g -Wall -I./ --std=c++0x
RM = /bin/rm -f
SRCS = $(wildcard src/*.cpp) $(wildcard src/*.c)  
OBJS = $(patsubst %.cpp,%.o,$(patsubst %.c,%.o ,$(SRCS)))
DEPS = $(patsubst %d,%o,$(OBJS))
PROG = lispy.tsk

all: $(PROG)

$(PROG): $(OBJS) $(DEPS)
	$(LD) $(OBJS) -o $(PROG) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@
	
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
	
%.d: %.cpp
	@set -e; rm -f $@; \
	$(CXX) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
	
clean:
	$(RM) $(PROG) $(OBJS)
