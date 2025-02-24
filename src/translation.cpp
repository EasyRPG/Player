/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#include "translation.h"

// Headers
#include <fstream>
#include <iomanip>
#include <memory>
#include <lcf/data.h>
#include <lcf/rpg/terms.h>
#include <lcf/rpg/map.h>
#include "lcf/rpg/mapinfo.h"

#include "baseui.h"
#include "cache.h"
#include "font.h"
#include "main_data.h"
#include "game_actors.h"
#include "game_map.h"
#include "player.h"
#include "output.h"
#include "utils.h"
#include "scene.h"

// Name of the translate directory
#define TRDIR_NAME "language"

// Name of expected files
#define TRFILE_RPG_RT_LDB    "rpg_rt.ldb.po"
#define TRFILE_RPG_RT_COMMON "rpg_rt.ldb.common.po"
#define TRFILE_RPG_RT_BATTLE "rpg_rt.ldb.battle.po"
#define TRFILE_RPG_RT_LMT    "rpg_rt.lmt.po"
#define TRFILE_META_INI      "meta.ini"

// Message box commands to remove a message box or add one in place.
// These commands are added by translators in the .po files to manipulate
//   text boxes at runtime. They are magic strings that will not otherwise
//   appear in the source of EasyRPG, but they should not be deleted.
#define TRCUST_REMOVEMSG        "<easyrpg:delete_page>"
#define TRCUST_ADDMSG           "<easyrpg:new_page>"


FilesystemView Tr::GetTranslationFilesystem() {
	return Player::translation.GetRootTree();
}

bool Tr::HasActiveTranslation() {
	return !GetCurrentTranslationId().empty();
}

std::string Tr::GetCurrentTranslationId() {
	return Player::translation.GetCurrentLanguage().lang_dir;
}

std::string Tr::GetCurrentLanguageCode() {
	return Player::translation.GetCurrentLanguage().lang_code;
}

FilesystemView Tr::GetCurrentTranslationFilesystem() {
	return Player::translation.GetRootTree().Subtree(GetCurrentTranslationId());
}

void Translation::Reset()
{
	ClearTranslationLookups();

	translation_root_fs = FilesystemView();
	languages.clear();
	current_language = {};
	default_language = {};
}

void Translation::InitTranslations()
{
	// Reset
	Reset();

	// Determine if the "languages" directory exists, and convert its case.
	auto fs = FileFinder::Game();
	auto game_tree = fs.ListDirectory();
	for (const auto& tr_name : *game_tree) {
		if (tr_name.first == TRDIR_NAME) {
			translation_root_fs = fs.Subtree(tr_name.second.name);
			break;
		}
	}
	if (translation_root_fs) {
		// Now list all directories within the translate dir
		auto translation_tree = translation_root_fs.ListDirectory();

		// Now iterate over every subdirectory.
		for (const auto& tr_name : *translation_tree) {
			Language item;
			item.lang_dir = tr_name.second.name;
			item.lang_name = tr_name.second.name;

			// If there's a manifest file, read the language name and help text from that
			auto meta_file = translation_root_fs.OpenFile(item.lang_dir, TRFILE_META_INI);
			if (meta_file) {
				lcf::INIReader ini(meta_file);
				item.lang_name = ini.GetString("Language", "Name", item.lang_name);
				item.lang_desc = ini.GetString("Language", "Description", "");
				item.lang_code = ini.GetString("Language", "Code", "");
				item.lang_term = ini.GetString("Language", "Term", "Language");
				item.game_title = ini.GetString("Language", "GameTitle", "");
				item.use_builtin_font = Utils::LowerCase(ini.GetString("Language", "Font", "")) == "builtin";

				if (item.lang_dir == "default") {
					// Metadata for the normal game language
					default_language = item;
					continue;
				}
			}

			languages.push_back(item);
		}
	}
}

const Language& Translation::GetCurrentLanguage() const
{
	return current_language;
}

const Language& Translation::GetDefaultLanguage() const
{
	return default_language;
}

FilesystemView Translation::GetRootTree() const
{
	return translation_root_fs;
}

bool Translation::HasTranslations() const
{
	return !languages.empty();
}

const std::vector<Language>& Translation::GetLanguages() const
{
	return languages;
}


void Translation::SelectLanguage(StringView lang_id)
{
	// Try to read in our language files.
	Output::Debug("Changing language to: '{}'", (!lang_id.empty() ? lang_id : "<Default>"));

	AsyncHandler::ClearRequests();

	if (!lang_id.empty()) {
		auto root = GetRootTree();
		if (!root) {
			Output::Warning("Cannot load translation. 'Language' folder does not exist");
			return;
		}

		FilesystemView language_tree = root.Subtree(lang_id);
		if (language_tree) {
			auto files = Utils::MakeSvArray(TRFILE_RPG_RT_LDB, TRFILE_RPG_RT_BATTLE, TRFILE_RPG_RT_COMMON, TRFILE_RPG_RT_LMT, "Font/Font", "Font/Font2");
			request_counter = static_cast<int>(files.size());
			for (auto s: files) {
				FileRequestAsync* request = AsyncHandler::RequestFile(language_tree.GetFullPath(), s);
				request->SetImportantFile(true);
				requests.emplace_back(request->Bind(&Translation::SelectLanguageAsync, this, lang_id));
				request->Start();
			}
		} else {
			Output::Warning("Translation for '{}' does not appear to exist", lang_id);
		}
	} else {
		// Default language, no request needed
		request_counter = 1;
		SelectLanguageAsync(nullptr, lang_id);
	}
}

void Translation::SelectLanguageAsync(FileRequestResult*, StringView lang_id) {
	--request_counter;
	if (request_counter == 0) {
		requests.clear();
	} else {
		// Waiting for remaining callbacks
		return;
	}

	if (!ParseLanguageFiles(lang_id)) {
		return;
	}

	// We reload the entire database as a precaution.
	Player::LoadDatabase();

	// Translation could provide custom fonts
	if (current_language.use_builtin_font) {
		Font::ResetDefault();
	} else {
		Player::LoadFonts();
	}

	// Rewrite our database+messages (unless we are on the Default language).
	// Note that map Message boxes are changed on map load, to avoid slowdown here.
	if (!current_language.lang_dir.empty()) {
		RewriteDatabase();
		RewriteTreemapNames();
		RewriteBattleEventMessages();
		RewriteCommonEventMessages();
	}

	if (!current_language.game_title.empty()) {
		Player::UpdateTitle(current_language.game_title);
	} else if (!Player::game_title_original.empty()) {
		Player::UpdateTitle(Player::game_title_original);
	}

	// Reset the cache, so that all images load fresh.
	Cache::Clear();

	Scene::instance->OnTranslationChanged();
}

void Translation::RequestAndAddMap(int map_id) {
	if (current_language.lang_dir.empty()) {
		return;
	}

	std::stringstream ss;
	ss << "Map" << std::setfill('0') << std::setw(4) << map_id << ".po";
	std::string map_name = ss.str();

	if (maps.find(Utils::LowerCase(map_name)) != maps.end()) {
		// Already loaded
		return;
	}

	FileRequestAsync* request = AsyncHandler::RequestFile(Tr::GetCurrentTranslationFilesystem().GetFullPath(), map_name);
	request->SetImportantFile(true);
	map_request = request->Bind([this, map_name](FileRequestResult*) {
		std::unique_ptr<Dictionary> dict = std::make_unique<Dictionary>();
		auto is = Tr::GetCurrentTranslationFilesystem().OpenInputStream(map_name);
		if (is) {
			ParsePoFile(std::move(is), *dict);
			maps[Utils::LowerCase(map_name)] = std::move(dict);
			Output::Debug("Loaded {} map .po file ({} map files loaded)", map_name, maps.size());
		}
	});
	request->Start();
}

bool Translation::ParseLanguageFiles(StringView lang_id)
{
	FilesystemView language_tree;

	// Create the directory tree (for lookups).
	if (!lang_id.empty()) {
		language_tree = GetRootTree().Subtree(lang_id);
		if (!language_tree) {
			Output::Warning("Translation for '{}' does not appear to exist", lang_id);
			return false;
		}
	}

	// Clear the old translation
	ClearTranslationLookups();

	// For default, this is all we need.
	if (!language_tree) {
		current_language = {};
		return true;
	}

	// Scan for files in the directory and parse them.
	for (const auto& tr_name : *language_tree.ListDirectory()) {
		if (tr_name.second.type != DirectoryTree::FileType::Regular) {
			continue;
		}

		if (tr_name.first == TRFILE_RPG_RT_LDB) {
			sys = std::make_unique<Dictionary>();
			auto is = language_tree.OpenInputStream(tr_name.second.name);
			if (is) {
				ParsePoFile(std::move(is), *sys);
			}
		} else if (tr_name.first == TRFILE_RPG_RT_BATTLE) {
			battle = std::make_unique<Dictionary>();
			auto is = language_tree.OpenInputStream(tr_name.second.name);
			if (is) {
				ParsePoFile(std::move(is), *battle);
			}
		} else if (tr_name.first == TRFILE_RPG_RT_COMMON) {
			common = std::make_unique<Dictionary>();
			auto is = language_tree.OpenInputStream(tr_name.second.name);
			if (is) {
				ParsePoFile(std::move(is), *common);
			}
		} else if (tr_name.first == TRFILE_RPG_RT_LMT) {
			mapnames = std::make_unique<Dictionary>();
			auto is = language_tree.OpenInputStream(tr_name.second.name);
			if (is) {
				ParsePoFile(std::move(is), *mapnames);
			}
		} else if (StringView(tr_name.first).ends_with(".po")) {
			// This will fail in the web player but is intentional
			// The fetching happens on map load instead
			// Still parsing all files locally to get syntax errors early
			auto is = language_tree.OpenInputStream(tr_name.second.name);
			if (is) {
				std::unique_ptr<Dictionary> dict = std::make_unique<Dictionary>();
				ParsePoFile(std::move(is), *dict);
				maps[tr_name.first] = std::move(dict);
			}
		}
	}

	auto it = std::find_if(languages.begin(), languages.end(), [&lang_id](const auto& lang) {
		return lang_id == lang.lang_dir;
	});
	assert(it != languages.end());
	current_language = *it;

	// Log
	Output::Debug("Translation loaded {} sys, {} common, {} battle, and {} map .po files", (sys==nullptr?0:1), (battle==nullptr?0:1), (common==nullptr?0:1), maps.size());

	return true;
}


void Translation::RewriteDatabase()
{
	if (!sys) {
		return;
	}

	lcf::rpg::ForEachString(lcf::Data::data, [this](lcf::DBString& value, auto& ctxt) {
		// When we re-write the database, we only care about translations that are exactly one level deep.
		if (ctxt.parent==nullptr || ctxt.parent->parent!=nullptr) {
			return;
		}

		// Look up the indexed form first; e.g., "actors.1.name", starting from 1 instead of 0
		if (ctxt.index >= 0) {
			if (sys->TranslateString<lcf::DBString>(fmt::format("{}.{}.{}", ctxt.parent->name, ctxt.parent->index+1, ctxt.name), value)) {
				return;
			}
		}

		// Look up the non-indexed form second; e.g., "actors.name"
		if (sys->TranslateString<lcf::DBString>(fmt::format("{}.{}", ctxt.parent->name, ctxt.name), value)) {
			return;
		}

		// Finally, look up a context-free form (just by value).
		if (sys->TranslateString<lcf::DBString>("", value)) {
			return;
		}
	});
}

void Translation::RewriteTreemapNames()
{
	if (mapnames) {
		for (lcf::rpg::MapInfo& map : lcf::Data::treemap.maps) {
			mapnames->TranslateString<lcf::DBString>("", map.name);
		}
	}
}

void Translation::RewriteBattleEventMessages()
{
	// Rewrite all event commands on all pages.
	if (battle) {
		for (lcf::rpg::Troop& troop : lcf::Data::troops) {
			for (lcf::rpg::TroopPage& page : troop.pages) {
				RewriteEventCommandMessage(*battle, page.event_commands);
			}
		}
	}
}


void Translation::RewriteCommonEventMessages()
{
	// Rewrite all event commands on all pages.
	if (common) {
		for (lcf::rpg::CommonEvent& ev : lcf::Data::commonevents) {
			RewriteEventCommandMessage(*common, ev.event_commands);
		}
	}
}


namespace {
	/**
	 * Helper class for iterating over and rewriting event commands.
	 * Starts at index 0.
	 */
	class CommandIterator {
	public:
		CommandIterator(std::vector<lcf::rpg::EventCommand>& commands) : commands(commands) {}

		/** Returns true if the index is past the end of the command list */
		bool Done() const {
			return index >= commands.size();
		}

		/** Advance the index through the command list by 1 */
		void Advance() {
			index += 1;
		}

		/** Retrieve the code of the EventCommand at the index */
		lcf::rpg::EventCommand::Code CurrentCmdCode() const {
			return static_cast<lcf::rpg::EventCommand::Code>(commands[index].code);
		}

		/** Retrieve the string of the EventCommand at the index */
		lcf::DBString& CurrentCmdString() const {
			return commands[index].string;
		}

		/** Retrieve the indent level of the EventCommand at the index */
		int CurrentCmdIndent() const {
			return commands[index].indent;
		}

		/** Retrieve parameter at position 'pos' of the EventCommand at the current index, or the devValue if no such parameter exists. */
		int CurrentCmdParam(size_t pos, int defVal = 0) const {
			if (pos < commands[index].parameters.size()) {
				return commands[index].parameters[pos];
			}
			return defVal;
		}

		/** Returns true if the current Event Command is ShowMessage */
		bool CurrentIsShowMessage() const {
			return CurrentCmdCode() == lcf::rpg::EventCommand::Code::ShowMessage;
		}

		/** Returns true if the current Event Command is ShowMessage_2 */
		bool CurrentIsShowMessage2() const {
			return CurrentCmdCode() == lcf::rpg::EventCommand::Code::ShowMessage_2;
		}

		/** Returns true if the current Event Command is ShowChoice */
		bool CurrentIsShowChoice() const {
			return CurrentCmdCode() == lcf::rpg::EventCommand::Code::ShowChoice;
		}

		/** Returns true if the current Event Command is ShowChoiceOption */
		bool CurrentIsShowChoiceOption() const {
			return CurrentCmdCode() == lcf::rpg::EventCommand::Code::ShowChoiceOption;
		}

		/** Returns true if the current Event Command is ShowChoiceEnd */
		bool CurrentIsShowChoiceEnd() const {
			return CurrentCmdCode() == lcf::rpg::EventCommand::Code::ShowChoiceEnd;
		}

		/** Returns true if the current Event Command is ChangeHeroName */
		bool CurrentIsChangeHeroName() const {
			return CurrentCmdCode() == lcf::rpg::EventCommand::Code::ChangeHeroName;
		}

		/** Returns true if the current Event Command is ChangeHeroTitle */
		bool CurrentIsChangeHeroTitle() const {
			return CurrentCmdCode() == lcf::rpg::EventCommand::Code::ChangeHeroTitle;
		}

		/** Returns true if the current Event Command is ChangeHeroTitle */
		bool CurrentIsConditionActorName() const {
			return CurrentCmdCode() == lcf::rpg::EventCommand::Code::ConditionalBranch &&
				CurrentCmdParam(0) == 5 && CurrentCmdParam(2) == 1;
		}

		/** Returns true if the current Event Command is ShowStringPicture */
		bool CurrentIsShowStringPicture() const {
			return CurrentCmdCode() == lcf::rpg::EventCommand::Code::Maniac_ShowStringPicture;
		}

		/**
		 * Add each line of a [ShowMessage,ShowMessage_2,...] chain to "msg_str" (followed by a newline)
		 * and save to "indexes" the index of each ShowMessage(2) command that was used to populate this
		 * (for rewriting later).
		 * Advances the index until after the last ShowMessage(2) command
		 */
		void BuildMessageString(std::stringstream& msg_str, std::vector<size_t>& indexes) {
			// No change if we're not on the right command.
			if (Done() || !CurrentIsShowMessage()) {
				return;
			}

			// Add the first line
			msg_str << CurrentCmdString() <<"\n";
			indexes.push_back(index);
			Advance();

			// Build lines 2 through 4
			while (!Done() && CurrentIsShowMessage2()) {
				msg_str << CurrentCmdString() <<"\n";
				indexes.push_back(index);
				Advance();
			}
		}

		/**
		 * Add each line of a [ShowChoice,ShowChoiceOption,...,ShowChoiceEnd] chain to "msg_str" (followed by a newline)
		 * and save to "indexes" the index of each ShowChoiceOption command that was used to populate this
		 * (for rewriting later).
		 * Advances the index until after the (first) ShowChoice command (but it will likely still be on a ShowChoiceOption/End)
		 */
		void BuildChoiceString(std::stringstream& msg_str, std::vector<size_t>& indexes) {
			// No change if we're not on the right command.
			if (Done() || !CurrentIsShowChoice()) {
				return;
			}

			// Advance to ChoiceOptions
			Advance();
			if(Done()) {
				return;
			}

			// Choices must be on the same indent level.
			// We have to save/restore the index, though, in the rare case that we skip something that can be translated.
			int indent = CurrentCmdIndent();
			size_t savedIndex = index;
			while (!Done()) {
				if (indent == CurrentCmdIndent()) {
					// Handle a new index
					if (CurrentIsShowChoiceOption() && CurrentCmdParam(0,0) < 4) {
						msg_str << CurrentCmdString() <<"\n";
						indexes.push_back(index);
					}

					// Done?
					if (CurrentIsShowChoiceEnd()) {
						break;
					}
				}
				Advance();
			}
			index = savedIndex;
		}

		/** Change the string value of the EventCommand at position "idx" to "newStr" */
		void ReWriteString(size_t idx, StringView newStr) {
			if (idx < commands.size()) {
				commands[idx].string = lcf::DBString(newStr);
			}
		}

		/**
		 * Puts a "ShowMessage" or "ShowMessage_2" command into the command stack before position "idx".
		 * Sets the string value to "line". Note that ShowMessage_2 is chosen if baseMsgBox is false.
		 * This also updates the index if relevant, but it does not update external index caches.
		 */
		void PutShowMessageBeforeIndex(StringView line, size_t idx, bool baseMsgBox) {
			// We need a reference index for the indent.
			size_t refIndent = 0;
			if (idx < commands.size()) {
				refIndent = commands[idx].indent;
			} else if (idx == commands.size() && commands.size()>0) {
				refIndent = commands[idx-1].indent;
			}

			if (idx <= commands.size()) {
				lcf::rpg::EventCommand newCmd;
				newCmd.code = static_cast<int>(baseMsgBox ? lcf::rpg::EventCommand::Code::ShowMessage : lcf::rpg::EventCommand::Code::ShowMessage_2);
				newCmd.indent = refIndent;
				newCmd.string = lcf::DBString(line);
				commands.insert(commands.begin()+idx, newCmd);

				// Update our index
				if (index >= idx) {
					index += 1;
				}
			}
		}

		/**
		 * Remove the EventCommand at position "idx" from the command stack.
		 * Also updates our index, if relevant.
		 */
		void RemoveByIndex(size_t idx) {
			if (idx < commands.size()) {
				commands.erase(commands.begin() + idx);

				// Update our index
				if (index > idx) {
					index -= 1;
				}
			}
		}

		/**
		 * Add multiple message boxes to the command stack before "idx".
		 * The "msgs" each represent lines in new, independent message boxes (so they will have both ShowMessage and ShowMessage_2)
		 * Also updates our index, if relevant.
		 */
		void InsertMultiMessageBefore(std::vector<std::vector<std::string>>& msgs, size_t idx) {
			for (std::vector<std::string>& lines : msgs) {
				while (!lines.empty()) {
					PutShowMessageBeforeIndex(lines.back(), idx, lines.size()==1);
					lines.pop_back();
				}
			}
		}

	private:
		std::vector<lcf::rpg::EventCommand>& commands;
		size_t index = 0;
	};
}



std::vector<std::vector<std::string>> Translation::TranslateMessageStream(const Dictionary& dict, const std::stringstream& msg, char trimChar) {
	// Prepare source string
	std::string msgStr = msg.str();
	if (msgStr.size()>0 && msgStr.back() == trimChar) {
		msgStr.pop_back();
	}

	// Translation exists?
	std::vector<std::vector<std::string>> res;
	if (dict.TranslateString<std::string>("", msgStr)) {
		// First, get all lines.
		std::vector<std::string> lines = Utils::Tokenize(msgStr, [](char32_t c) { return c=='\n'; });

		// Now, break into message boxes based on the ADDMSG string
		res.push_back(std::vector<std::string>());
		for (const auto& line : lines) {
			if (line == TRCUST_ADDMSG) {
				res.push_back(std::vector<std::string>());
			} else {
				res.back().push_back(line);
			}

			// Special case: stop once you've found a REMMSG (to avoid the case where both add/rem are present)
			if (line == TRCUST_REMOVEMSG) {
				break;
			}
		}

		// Ensure we never get an empty vector (force using the REMMSG command)
		for (std::vector<std::string>& msgbox : res) {
			if (msgbox.empty()) {
				msgbox.push_back("");
			}
		}
	}

	return res;
}


void Translation::RewriteEventCommandMessage(const Dictionary& dict, std::vector<lcf::rpg::EventCommand>& commandsOrig) {
	// A note on this function: it is (I feel) necessarily complicated, given what it's actually doing.
	// I've tried to abstract most of this complexity away behind an "iterator" interface, so that we do not
	// have to track the current index directly in this function.
	CommandIterator commands(commandsOrig);
	while (!commands.Done()) {
		// We only need to deal with either Message or Choice commands
		if (commands.CurrentIsShowMessage()) {
			// Build up the lines of Message texts
			std::stringstream msg_str;
			std::vector<size_t> msg_indexes;
			commands.BuildMessageString(msg_str, msg_indexes);

			// Go through messages first, including possible choices
			if (msg_indexes.size()>0) {
				// Get our lines, possibly including "combined"
				std::vector<std::vector<std::string>> msgs = TranslateMessageStream(dict, msg_str, '\n');
				if (msgs.size()>0) {
					// The complex replacement logic is based on the last message box, then all remaining things are simply left back in.
					std::vector<std::string>& lines = msgs.back();

					// There is a special case here: if we are asked to remove a message box, we should do nothing further
					// This command is *only* respected as the first line of a message box.
					if (lines[0]==TRCUST_REMOVEMSG) {
						// Clear all message boxes in reverse order.
						while (!msg_indexes.empty()) {
							commands.RemoveByIndex(msg_indexes.back());
							msg_indexes.pop_back();
						}
					} else {
						// Trim lines down to allowed remaining (with choices).
						const size_t maxLines = 4;
						while (lines.size() > maxLines) {
							lines.pop_back();
						}

						// First, pop extra entries from the back of msg_cmd; this preserves the remaining entries' indexes
						while (msg_indexes.size() > lines.size()) {
							commands.RemoveByIndex(msg_indexes.back());
							msg_indexes.pop_back();
						}

						// Next, push extra entries from the back of lines to msg_cmd
						while (lines.size() > msg_indexes.size()) {
							commands.PutShowMessageBeforeIndex("", msg_indexes.back()+1, false);
							msg_indexes.push_back(msg_indexes.back()+1);
						}

						// Now simply go through each entry and update it.
						for (size_t num=0; num<msg_indexes.size(); num++) {
							commands.ReWriteString(msg_indexes[num], lines[num]);
						}
					}
					msgs.pop_back();

					// Now add remianing messages, if any
					commands.InsertMultiMessageBefore(msgs, msg_indexes[0]);
				}
			}

			// Note that commands.Advance() has already happened within the above code.
		} else if (commands.CurrentIsShowChoice()) {
			// Build up the lines of Choice elements
			std::stringstream choice_str;
			std::vector<size_t> choice_indexes; // Number of entries == number of choices
			commands.BuildChoiceString(choice_str, choice_indexes);

			// Go through choices.
			if (choice_indexes.size() > 0) {
				// Translate, break back into lines.
				std::vector<std::vector<std::string>> msgs = TranslateMessageStream(dict, choice_str, '\n');
				if (msgs.size() > 0) {
					// Logic here is also based on the last message box.
					std::vector<std::string> &lines = msgs.back();

					// We only pick the first X entries from the translation, since we can't change the Choice count.
					for (size_t num = 0; num < choice_indexes.size() && num < lines.size(); num++) {
						commands.ReWriteString(choice_indexes[num], lines[num]);
					}

					msgs.pop_back();

					// Now add remianing messages, if any
					commands.InsertMultiMessageBefore(msgs, choice_indexes[0]);
				}
			}

			// Note that commands.Advance() has already happened within the above code.
		} else if (commands.CurrentIsChangeHeroName() || commands.CurrentIsConditionActorName()) {
			dict.TranslateString("actors.name", commands.CurrentCmdString());
			commands.Advance();
		} else if (commands.CurrentIsChangeHeroTitle()) {
			dict.TranslateString("actors.title", commands.CurrentCmdString());
			commands.Advance();
		} else if (commands.CurrentIsShowStringPicture()) {
			auto cmdstr = StringView(commands.CurrentCmdString());
			if (!cmdstr.empty() && cmdstr[0] == '\x01') {
				size_t escape_idx = 1;
				for (escape_idx = 1; escape_idx < cmdstr.size(); ++escape_idx) {
					char c = cmdstr[escape_idx];
					if (c == '\x01' || c == '\x02' || c == '\x03') {
						break;
					}
				}

				// String Picture use \r\n linebreaks
				// Rewrite them to \n
				std::string term = Utils::ReplaceAll(ToString(cmdstr.substr(1, escape_idx - 1)), "\r\n", "\n");
				dict.TranslateString("strpic", term);
				// Reintegrate the term
				commands.CurrentCmdString() = lcf::DBString("\x01" + term + ToString(cmdstr.substr(escape_idx)));
			}

			commands.Advance();
		} else {
			commands.Advance();
		}
	}
}

void Translation::RewriteMapMessages(StringView map_name, lcf::rpg::Map& map) {
	// Retrieve lookup for this map.
	auto mapIt = maps.find(ToString(map_name));
	if (mapIt==maps.end()) { return; }

	// Rewrite all event commands on all pages.
	for (lcf::rpg::Event& ev : map.events) {
		for (lcf::rpg::EventPage& pg : ev.pages) {
			RewriteEventCommandMessage(*mapIt->second, pg.event_commands);
		}
	}
}

void Translation::ParsePoFile(Filesystem_Stream::InputStream is, Dictionary& out)
{
	if (is) {
		Dictionary::FromPo(out, is);
	}
}

void Translation::ClearTranslationLookups()
{
	sys.reset();
	common.reset();
	battle.reset();
	mapnames.reset();
	maps.clear();
}

//////////////////////////////////////////////////////////
// NOTE: The code from here on out is duplicated in LcfTrans.
//       At some point it should be merged to a common location.
//////////////////////////////////////////////////////////


void Dictionary::addEntry(const Entry& entry)
{
	// Space-saving measure: If the translation string is empty, there's no need to save it (since we will just show the original).
	if (!entry.translation.empty()) {
		entries[entry.context][entry.original] = entry.translation;
	}
}

// Returns success
void Dictionary::FromPo(Dictionary& res, Filesystem_Stream::InputStream& in) {
	std::string line;
	lcf::StringView line_view;
	bool found_header = false;
	bool parse_item = false;
	int line_number = 0;

	Entry e;

	auto extract_string = [&](size_t offset) -> std::string {
		if (offset >= line_view.size()) {
			Output::Error("{}\n\nParse error (Line {}) is empty", FileFinder::GetPathInsideGamePath(in.GetName()), line_number);
			return "";
		}

		std::stringstream out;
		bool slash = false;
		bool first_quote = false;

		for (char c : line_view.substr(offset)) {
			if (!first_quote) {
				if (c == ' ') {
					continue;
				} else if (c == '"') {
					first_quote = true;
					continue;
				}
				Output::Error("{}\n\nParse error (Line {}): Expected \", got \"{}\":\n{}", FileFinder::GetPathInsideGamePath(in.GetName()), line_number, c, line);
				return "";
			}

			if (!slash && c == '\\') {
				slash = true;
			} else if (slash) {
				slash = false;
				switch (c) {
					case '\\':
						out << c;
						break;
					case 'n':
						out << '\n';
						break;
					case '"':
						out << '"';
						break;
					default: {
						Output::Error("{}\n\nParse error (Line {}): Expected \\, \\n or \", got \"{}\":\n{}", FileFinder::GetPathInsideGamePath(in.GetName()), line_number, c, line);
						break;
					}
				}
			} else {
				// no-slash
				if (c == '"') {
					// done
					return out.str();
				}
				out << c;
			}
		}

		Output::Error("{}\n\nParse error (Line {}): Unterminated line:\n{}", FileFinder::GetPathInsideGamePath(in.GetName()), line_number, line);
		return out.str();
	};

	auto read_msgstr = [&]() {
		// Parse multiply lines until empty line or comment
		e.translation = extract_string(6);

		while (Utils::ReadLine(in, line)) {
			line_view = Utils::TrimWhitespace(line);
			++line_number;
			if (line_view.empty() || line_view.starts_with("#")) {
				break;
			}
			e.translation += extract_string(0);
		}

		parse_item = false;
		res.addEntry(e);
		e = Entry();
	};

	auto read_msgid = [&]() {
		// Parse multiply lines until empty line or msgstr is encountered
		e.original = extract_string(5);

		while (Utils::ReadLine(in, line)) {
			line_view = Utils::TrimWhitespace(line);
			++line_number;
			if (line_view.empty() || line_view.starts_with("msgstr")) {
				read_msgstr();
				return;
			}
			e.original += extract_string(0);
		}
	};

	while (Utils::ReadLine(in, line)) {
		line_view = Utils::TrimWhitespace(line);
		++line_number;
		if (!found_header) {
			if (line_view.starts_with("msgstr")) {
				found_header = true;
			}
			continue;
		}

		if (!parse_item) {
			if (line_view.starts_with("msgctxt")) {
				e.context = extract_string(7);

				parse_item = true;
			} else if (line_view.starts_with("msgid")) {
				parse_item = true;
				read_msgid();
			}
		} else {
			if (line_view.starts_with("msgid")) {
				read_msgid();
			} else if (line_view.starts_with("msgstr")) {
				read_msgstr();
			}
		}
	}
}
