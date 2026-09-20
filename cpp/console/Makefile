CXX      ?= c++
CXXFLAGS ?= -O2 -Wall -Wextra -std=c++11
SRCDIR    = ../stetris
TARGET    = stetris

OBJS = stconsolemain.o \
       stconsoleview.o \
       stplay.o \
       stblock.o \
       stblocks.o \
       stboard.o \
       stscore.o \
       strandom.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
