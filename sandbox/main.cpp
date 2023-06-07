#include "Sandbox.h"

int main(int arg_count, char* args[]) {
	dvig::AppSpec spec;
	spec.arg_count = arg_count;
	spec.args = args;
	spec.fixed_ups = 50.0f;
	Sandbox* sandbox = new Sandbox(spec);
	sandbox->run();
	delete sandbox;
	return 0;
}
