OBJS = src/ClassLoader.o \
	   src/JVM.o \
	   src/main.o \
	   src/Stack.o \
	   src/Opcodes.o \
	   src/ConstantPool.o

CXXFLAGS=-Iinclude -g -fsanitize=undefined -MMD -MP -std=c++20 -Wall

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