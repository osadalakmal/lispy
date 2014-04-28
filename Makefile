CXX:=g++
CC:=gcc
LIBRARY_PATH=$(shell echo "/usr/lib/$(gcc -print-multiarch)")
C_INCLUDE_PATH=$(shell echo "/usr/include/$(gcc -print-multiarch)")
CPLUS_INCLUDE_PATH=$(shell echo "/usr/include/$(gcc -print-multiarch)")
LDFLAGS := -lreadline
LD = g++
CFLAGS = -g -Wall -I./
CPPFLAGS = -g -Wall -I./ -I/home/osada/progs/boost_1_55_0 --std=c++0x 
RM = /bin/rm -f
SRCS = $(wildcard src/*.c) $(wildcard src/*.cpp)  
OBJS = $(patsubst %.cpp,%.o,$(patsubst %.c,%.o ,$(SRCS)))
DEPS = $(OBJS:.o=.d)
PROG = lispy.tsk
-include $(DEPS)

all: $(PROG)

$(PROG): $(OBJS)
	$(LD) $(OBJS) -o $(PROG) $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $< -o $*.o
	$(CXX) -MM $(CPPFLAGS) $< > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp
	
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $*.o
	$(CC) -MM $(CFLAGS) $< > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp
	
clean:
	$(RM) $(PROG) $(OBJS) $(DEPS)
