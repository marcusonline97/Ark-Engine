#include "ArkApp.h"
#include "Logger.h"

int main()
{
	// Enable logging levels
    Logging::EnableLevel(Logging::Level::INIT);
    Logging::EnableLevel(Logging::Level::DEBUG);
    Logging::EnableLevel(Logging::Level::_ERROR);
    Logging::EnableLevel(Logging::Level::FATAL);
    Logging::EnableLevel(Logging::Level::TODO);
    Logging::EnableLevel(Logging::Level::WARNING);
    Logging::EnableLevel(Logging::Level::FUNCTION);
    Logging::ToDo() << "Logger Initialized";
    Logging::ToDo() << "Starting Engine";
    try
    {
        App app;
        app.Run();
    }
    catch (const std::exception& e)
    {
        Logging::Fatal() << "Fatal error: %s\n", e.what();
        return -1;
    }

	Logging::Shutdown();
    return 0;

}
