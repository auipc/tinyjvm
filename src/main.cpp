#include <tinyjvm/classloader.h>
#include <tinyjvm/jvm.h>

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Usage: %s [MainClass] [ClassFile.class]\n", argv[0]);
		return 1;
	}

	ClassLoader *cl = new ClassLoader(std::string(argv[1]), std::string(argv[2]));
	cl->load_class();
	JVM *jvm = new JVM(cl);
	jvm->run();
	int exitcode = jvm->exitcode();
	delete jvm;
	delete cl;
	return exitcode;
}
