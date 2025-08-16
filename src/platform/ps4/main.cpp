#include "player.h"
#include "output.h"

static void log(LogLevel lvl, std::string const& message, LogCallbackUserData userdata)
{

}

int main(int argc, char* argv[])
{
	std::vector<std::string> args(argv, argv + argc);
	
	args.push_back("--project-path");
	args.push_back("/data/easyrpg-player");

	Output::SetLogCallback(log, nullptr);
	
	Player::Init(std::move(args));
	
	Player::Run();
	
	while(1)
	{
	
	}
	
	return Player::exit_code;
}
