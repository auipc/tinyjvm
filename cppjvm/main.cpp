#include <cppjvm/classloader.h>
#include <cppjvm/jvm.h>

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: %s [ClassFile.class]\n", argv[0]);
		return 1;
	}

	ClassLoader *cl = new ClassLoader("Program", std::string(argv[1]));
	cl->load_class();
	JVM *jvm = new JVM(cl);
	jvm->run();
	delete jvm;
	delete cl;
	return 0;
}
