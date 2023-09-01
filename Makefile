CXX=clang++
OBJS = src/classloader.o \
	   src/jvm.o \
	   src/main.o \
	   src/stack.o \
	   src/opcodes.o

CXXFLAGS=-Iinclude -g -fsanitize=undefined -MMD -MP -std=c++20

TINYJVM=jvm

all: $(TINYJVM)

run: $(TINYJVM)
	./$(TINYJVM) $(PROGRAM_ARGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TINYJVM): $(OBJS) 
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) 

-include $(OBJS:%.o=%.d)

clean:
	rm -f $(OBJS) $(OBJS:%.o=%.d)
