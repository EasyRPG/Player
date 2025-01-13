#include "doctest.h"
#include "game_destiny.h"
#include <vector>


TEST_SUITE_BEGIN("Game_Destiny");


static lcf::rpg::EventCommand MakeCommand(
	const lcf::rpg::EventCommand::Code code,
	const std::string& string
)
{
	lcf::rpg::EventCommand cmd;
	lcf::DBString dbStr(string);

	cmd.code = static_cast<uint32_t>(code);
	cmd.string = dbStr;

	return cmd;
}

static lcf::rpg::SaveEventExecFrame MakeFrame(
	std::vector<std::string>::const_iterator begin,
	std::vector<std::string>::const_iterator end
)
{
	lcf::rpg::SaveEventExecFrame frame;
	lcf::rpg::EventCommand::Code code;

	code = lcf::rpg::EventCommand::Code::Comment;

	while (begin != end)
	{
		const std::string& str = *begin++;

		frame.commands.push_back(MakeCommand(code, str));
		code = lcf::rpg::EventCommand::Code::Comment_2;
	}

	return frame;
}


TEST_CASE("AssertDestinyScript")
{
	Game_Destiny destiny;
	std::vector<std::string> lines {
		"$",
		"v[1] = 10;",
	};
	const char* destinyScript;

	auto frame = MakeFrame(lines.begin(), lines.end());
	destinyScript = destiny.Interpreter().MakeString(frame);

	CHECK_EQ(*destinyScript, '$');

	destiny.Interpreter().FreeString();
}

TEST_SUITE_END();
