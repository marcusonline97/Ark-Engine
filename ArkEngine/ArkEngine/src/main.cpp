#include "Initializer.h"

int main()
{
	if(!Init::StartUp())
		return -1;

	const int rc = Init::Run();
	Init::ShutDown();
	return rc;
}
