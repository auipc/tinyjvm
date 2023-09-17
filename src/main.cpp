#include <tinyjvm/ClassLoader.h>
#include <tinyjvm/JVM.h>

using namespace tinyJVM;

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Usage: %s [MainClass] [ClassFile.class]\n", argv[0]);
		return 1;
	}

	ClassLoader *cl =
		new ClassLoader(std::string(argv[1]), std::string(argv[2]));

	try {
		cl->load_class();
	} catch (ClassParseException e) {
		std::cout << "Invalid class file: " << e.what() << "\n";
		return 1;
	}

	JVM *jvm = new JVM(cl);
	jvm->run();
	int exitcode = jvm->exitcode();
	delete jvm;
	delete cl;
	return exitcode;
}
