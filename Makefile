CXX=clang++
OBJS = cppjvm/classloader.o \
	   cppjvm/jvm.o \
	   cppjvm/main.o \
	   cppjvm/stack.o

CXXFLAGS=-I. -g -fsanitize=undefined

CPPJVM=jvm

all: $(CPPJVM)

run: $(CPPJVM)
	./$(CPPJVM)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(CPPJVM): $(OBJS) 
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) 

-include $(OBJS:%.o=%.d)

clean:
	rm -f $(OBJS) $(OBJS:%.o=%.d)
