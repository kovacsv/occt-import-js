#include <iostream>

#include "occt-import-js.hpp"

int main (int argc, const char* argv[])
{
	if (argc < 2) {
		return 1;
	}

	StepToJson (argv[1]);

	return 0;
}
