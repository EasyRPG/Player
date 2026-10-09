#include "mock_game.h"
#include "game_dynrpg.h"
#include "game_interpreter.h"
#include "doctest.h"
#include <lcf/rpg/saveeventexecframe.h>

namespace {
using Cmd = lcf::rpg::EventCommand::Code;

lcf::rpg::EventCommand MakeCommand(Cmd code, std::vector<int32_t> params = {}, std::string text = "") {
	lcf::rpg::EventCommand com;
	com.code = static_cast<int32_t>(code);
	com.parameters = lcf::DBArray<int32_t>(params.begin(), params.end());
	com.string = lcf::DBString(text);
	return com;
}

lcf::rpg::EventCommand Comment(std::string text) {
	return MakeCommand(Cmd::Comment, {}, std::move(text));
}

class TestInterpreter : public Game_Interpreter {
public:
	using Game_Interpreter::GetFrame;

	/** Returns from the current frame, like a finished called event */
	void Pop() { _state.stack.pop_back(); }

	void Push(std::vector<lcf::rpg::EventCommand> list) {
		list.push_back(MakeCommand(Cmd::END));
		Game_Interpreter::Push<InterpreterExecutionType::Call, InterpreterEventType::CommonEvent>(std::move(list), 0);
	}

	/** Executes the commands of the current frame up to (excluding) index */
	void RunTo(int index) {
		while (GetFrame().current_command < index) {
			ExecuteCommand();
			++GetFrame().current_command;
		}
	}
};

/** Main_Data::game_dynrpg with DynRPG enabled, for the duration of a test */
struct DynRpgGuard {
	DynRpgGuard() {
		Player::game_config.patch_dynrpg.Set(true);
		Main_Data::game_dynrpg = std::make_unique<Game_DynRpg>();
		std::vector<int32_t> vars(10, 0);
		Main_Data::game_variables->SetData(vars);
		Main_Data::game_variables->SetWarning(0);
	}
	~DynRpgGuard() {
		Main_Data::game_dynrpg.reset();
	}
	Game_DynRpg& dyn() { return *Main_Data::game_dynrpg; }
};

// Control Variables: V[1] = 5
const std::vector<int32_t> set_v1 = {0, 1, 1, 0, 0, 5, 0};
// Control Variables: V[2] = 1
const std::vector<int32_t> set_v2 = {0, 2, 2, 0, 0, 1, 0};
}

TEST_SUITE_BEGIN("DynParams");

TEST_CASE("Overwrite is undone after execution") {
	const MockGame mg(MockMap::ePass40x30);
	DynRpgGuard g;
	TestInterpreter interp;

	interp.Push({
		Comment("@dynparams_add_param 6, 42"),
		Comment("@dynparams_overwrite_next"),
		MakeCommand(Cmd::ControlVars, set_v1),
		MakeCommand(Cmd::ControlVars, set_v2),
	});
	interp.RunTo(3);
	CHECK(Main_Data::game_variables->Get(1) == 42);

	// A command repeated until done (it returned false) keeps the overwrite
	Main_Data::game_variables->Set(1, 0);
	interp.GetFrame().current_command = 2;
	interp.RunTo(3);
	CHECK(Main_Data::game_variables->Get(1) == 42);

	// DynRPG restores the script line once executed
	interp.RunTo(4);
	CHECK(interp.GetFrame().commands[2].parameters[5] == 5);

	// Without a new overwrite the command runs as written
	interp.GetFrame().current_command = 2;
	interp.RunTo(3);
	CHECK(Main_Data::game_variables->Get(1) == 5);
}

TEST_CASE("Comment lines are overwritten too") {
	const MockGame mg(MockMap::ePass40x30);
	DynRpgGuard g;
	TestInterpreter interp;

	// Builds a comment command for another plugin
	interp.Push({
		Comment("@dynparams_append_string @easyrpg_add 1, 2, "),
		Comment("@dynparams_append_number 4"),
		Comment("@dynparams_overwrite_next"),
		Comment("@easyrpg_add 1, 0, 0"),
		Comment("next"),
	});
	interp.RunTo(5);
	CHECK(Main_Data::game_variables->Get(1) == 6);
	CHECK(interp.GetFrame().commands[3].string == "@easyrpg_add 1, 0, 0");
}

TEST_CASE("Comments are read as a whole") {
	const MockGame mg(MockMap::ePass40x30);
	DynRpgGuard g;
	TestInterpreter interp;

	interp.Push({
		// A note, its @ line is not a command
		Comment("Use the command"),
		MakeCommand(Cmd::Comment_2, {}, "@easyrpg_add 1, 2, 4"),
		// A command over several lines, empty ones included
		Comment("@easyrpg_add 2, 2,"),
		MakeCommand(Cmd::Comment_2, {}, ""),
		MakeCommand(Cmd::Comment_2, {}, "4"),
	});
	interp.RunTo(5);
	CHECK(Main_Data::game_variables->Get(1) == 0);
	CHECK(Main_Data::game_variables->Get(2) == 6);
}

TEST_CASE("State belongs to the script") {
	const MockGame mg(MockMap::ePass40x30);
	DynRpgGuard g;
	TestInterpreter a, b;

	a.Push({MakeCommand(Cmd::ControlVars, set_v1)});
	b.Push({MakeCommand(Cmd::ControlVars, set_v1)});

	g.dyn().Invoke("@dynparams_add_param 6, 9", &a);
	g.dyn().Invoke("@dynparams_overwrite_next", &a);

	// Another interpreter is unaffected
	CHECK(!g.dyn().OnEventCommand(b, b.GetFrame()));

	// So is a frame called by the script
	a.Push({MakeCommand(Cmd::ControlVars, set_v1)});
	CHECK(!g.dyn().OnEventCommand(a, a.GetFrame()));

	// Back in the script that built the overwrite
	a.Pop();
	auto original = g.dyn().OnEventCommand(a, a.GetFrame());
	REQUIRE(original);
	CHECK(original->parameters[5] == 5);
	CHECK(a.GetFrame().commands[0].parameters[5] == 9);
}

TEST_CASE("Pushing a frame drops the state left at that depth") {
	const MockGame mg(MockMap::ePass40x30);
	DynRpgGuard g;
	TestInterpreter a;

	a.Push({MakeCommand(Cmd::ControlVars, set_v1)});
	a.Push({MakeCommand(Cmd::ControlVars, set_v1)});
	g.dyn().Invoke("@dynparams_add_param 6, 9", &a);
	g.dyn().Invoke("@dynparams_overwrite_next", &a);

	a.Pop();
	a.Push({MakeCommand(Cmd::ControlVars, set_v1)});
	CHECK(!g.dyn().OnEventCommand(a, a.GetFrame()));
}

TEST_CASE("Parameters out of range are ignored") {
	const MockGame mg(MockMap::ePass40x30);
	DynRpgGuard g;
	TestInterpreter a;

	a.Push({MakeCommand(Cmd::ControlSwitches, {0, 1, 1, 0})});
	g.dyn().Invoke("@dynparams_add_param 2, 3", &a);
	g.dyn().Invoke("@dynparams_add_param 5, 1", &a);
	g.dyn().Invoke("@dynparams_add_param 0, 1", &a);
	g.dyn().Invoke("@dynparams_overwrite_next", &a);
	REQUIRE(g.dyn().OnEventCommand(a, a.GetFrame()));

	const auto& com = a.GetFrame().commands[0];
	REQUIRE(com.parameters.size() == 4);
	CHECK(com.parameters[1] == 3);
}

TEST_CASE("Message lines and choices") {
	const MockGame mg(MockMap::ePass40x30);
	DynRpgGuard g;
	TestInterpreter a;

	a.Push({
		MakeCommand(Cmd::ShowMessage, {}, "line 1"),
		MakeCommand(Cmd::ShowMessage_2, {}, "line 2"),
		MakeCommand(Cmd::ShowMessage_2, {}, "line 3"),
		MakeCommand(Cmd::ShowChoice, {0}, "a/b/c"),
		MakeCommand(Cmd::ShowChoiceOption, {0}, "a"),
		MakeCommand(Cmd::ShowChoice, {0}, "nested"),
		MakeCommand(Cmd::ShowChoiceOption, {0}, "n"),
		MakeCommand(Cmd::ShowChoiceEnd),
		MakeCommand(Cmd::ShowChoiceOption, {1}, "b"),
		MakeCommand(Cmd::ShowChoiceOption, {2}, "c"),
		MakeCommand(Cmd::ShowChoiceEnd),
	});
	g.dyn().Invoke(R"(@dynparams_message_line_append_string 1, "Level ")", &a);
	g.dyn().Invoke("@dynparams_message_line_append_number 1, 5", &a);
	g.dyn().Invoke(R"(@dynparams_message_line_append_string 2, "a ""quoted"" word")", &a);
	g.dyn().Invoke(R"(@dynparams_message_line_append_string 5, "ignored")", &a);
	g.dyn().Invoke(R"(@dynparams_choice_case_append_string 2, "Bomb (")", &a);
	g.dyn().Invoke("@dynparams_choice_case_append_number 2, 3", &a);
	g.dyn().Invoke(R"x(@dynparams_choice_case_append_string 2, ")")x", &a);
	g.dyn().Invoke("@dynparams_list_params", &a);
	g.dyn().Invoke("@dynparams_overwrite_next", &a);
	REQUIRE(g.dyn().OnEventCommand(a, a.GetFrame()));

	const auto& commands = a.GetFrame().commands;
	CHECK(commands[0].string == "Level 5");
	CHECK(commands[1].string == "a \"quoted\" word");
	// Lines without text are emptied
	CHECK(commands[2].string == "");
	// The choice after the message, skipping the nested one
	CHECK(commands[4].string == "a");
	CHECK(commands[6].string == "n");
	CHECK(commands[8].string == "Bomb (3)");
	CHECK(commands[9].string == "c");
}

TEST_CASE("Clear params") {
	const MockGame mg(MockMap::ePass40x30);
	DynRpgGuard g;
	TestInterpreter a;

	a.Push({MakeCommand(Cmd::ControlVars, set_v1)});
	g.dyn().Invoke("@dynparams_add_param 6, 9", &a);
	g.dyn().Invoke("@dynparams_overwrite_next", &a);
	g.dyn().Invoke("@dynparams_clear_params", &a);
	CHECK(!g.dyn().OnEventCommand(a, a.GetFrame()));
}

TEST_SUITE_END();
