
#include <restbed>

using namespace restbed;

class SimpleLogger : virtual public Logger
{
public:
	SimpleLogger() {};
	~SimpleLogger() {};
	void start(const std::shared_ptr< const restbed::Settings >& settings) {};
	void stop(void) {};

	void log(const Level level, const char* format, ...)
	{
		va_list argptr;
		va_start(argptr, format);
		vfprintf(stdout, format, argptr);
		va_end(argptr);
		printf("\n");
	};

	void log_if(bool expression, const Level level, const char* format, ...) {};
};