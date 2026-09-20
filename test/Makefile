CXX      ?= c++
CXXFLAGS ?= -O2 -Wall -Wextra -std=c++11
SRCDIR    = ../src
TARGET    = stlogictest

# Only the logic layer is linked, never a UI
OBJS = stlogictest.o \
       stplay.o \
       stblock.o \
       stblocks.o \
       stboard.o \
       stscore.o \
       strandom.o

all: $(TARGET)

test: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(SRCDIR) -c -o $@ $<

%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(SRCDIR) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all test clean
