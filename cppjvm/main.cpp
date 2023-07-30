#include <cppjvm/jvm.h>
#include <cppjvm/classloader.h>

int main() {
	ClassLoader *cl = new ClassLoader("Program.class");
	cl->load_class();
	JVM *jvm = new JVM(cl);
	jvm->run();
	delete jvm;
	delete cl;
}
