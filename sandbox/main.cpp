#include "Sandbox.h"

int main(int arg_count, char* args[]) {
	Sandbox* sandbox = new Sandbox;
	sandbox->run();
	delete sandbox;
	return 0;
}
