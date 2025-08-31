#include <orbis/libkernel.h>
#include <orbis/SystemService.h>
#include "player.h"
#include "output.h"

static char log_buffer[1024];
static const size_t log_buffer_size = sizeof(log_buffer)/sizeof(log_buffer[0]);

static void log(LogLevel lvl, std::string const& message, LogCallbackUserData userdata)
{
	(void)userdata;
	
#ifdef NDEBUG
	if( (LogLevel::Debug == lvl) || (LogLevel::Info == lvl) ) 
	{
		/* Disable debug and info messages in release */
		return;
	}
#endif

	snprintf(log_buffer, log_buffer_size, "<EasyRPGPlayer> %s\n", message.c_str());
	sceKernelDebugOutText(0, log_buffer);
}
int main(int argc, char* argv[])
{
	std::vector<std::string> args(argv, argv + argc);

	args.push_back("--project-path");
	args.push_back("/data/easyrpg-player");

	Output::SetLogCallback(log, nullptr);
	
	Player::Init(std::move(args));
	
	Player::Run();
	
	/* Prevent application from crashing */
	sceSystemServiceLoadExec("EXIT", NULL);
	
	return Player::exit_code;
}
