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

#include "cache.h"
#include "main_data.h"
#include "game_actors.h"
#include "game_map.h"
#include "player.h"
#include "output.h"
#include "utils.h"


// Name of the translate directory
#define TRDIR_NAME "languages"

// Name of expected files
#define TRFILE_RPG_RT_LDB    "rpg_rt.ldb.po"
#define TRFILE_RPG_RT_COMMON "rpg_rt.ldb.common.po"
#define TRFILE_RPG_RT_BATTLE "rpg_rt.ldb.battle.po"
#define TRFILE_RPG_RT_LMT    "rpg_rt.lmt.po"
#define TRFILE_META_INI      "META.INI"

// Message box commands to remove a message box or add one in place.
// These commands are added by translators in the .po files to manipulate
//   text boxes at runtime. They are magic strings that will not otherwise
//   appear in the source of EasyRPG, but they should not be deleted.
#define TRCUST_REMOVEMSG        "<easyrpg:delete_page>"
#define TRCUST_ADDMSG           "<easyrpg:new_page>"


DirectoryTreeView Tr::GetTranslationTree() {
	return Player::translation.GetRootTree();
}

std::string Tr::GetCurrentTranslationId() {
	return Player::translation.GetCurrentLanguageId();
}

DirectoryTreeView Tr::GetCurrentTranslationTree() {
	return Player::translation.GetRootTree().Subtree(GetCurrentTranslationId());
}


void Translation::Reset()
{
	ClearTranslationLookups();

	languages.clear();
	current_language = "";
}

void Translation::InitTranslations()
{
	// Reset
	Reset();

	// Determine if the "languages" directory exists, and convert its case.
	auto tree = FileFinder::GetDirectoryTree();
	translation_root_tree = tree.Subtree(TRDIR_NAME);
	if (translation_root_tree) {
		// Now list all directories within the translate dir
		auto translation_tree = translation_root_tree.ListDirectory();

		// Now iterate over every subdirectory.
		for (const auto& tr_name : *translation_tree) {
			Language item;
			item.lang_dir = tr_name.second.name;
			item.lang_name = tr_name.second.name;

			// If there's a manifest file, read the language name and help text from that.
			std::string meta_name = translation_root_tree.FindFile(item.lang_dir, TRFILE_META_INI);
			if (!meta_name.empty()) {
				lcf::INIReader ini(meta_name);
				item.lang_name = ini.GetString("Language", "Name", item.lang_name);
				item.lang_desc = ini.GetString("Language", "Description", "");
			}

			languages.push_back(item);
		}
	}
}

std::string Translation::GetCurrentLanguageId() const
{
	return current_language;
}

DirectoryTreeView Translation::GetRootTree() const
{
	return translation_root_tree;
}

bool Translation::HasTranslations() const
{
	return !languages.empty();
}

const std::vector<Language>& Translation::GetLanguages() const
{
	return languages;
}


void Translation::SelectLanguage(const std::string& lang_id)
{
	// Try to read in our language files.
	Output::Debug("Changing language to: '{}'", (!lang_id.empty() ? lang_id : "<Default>"));
	if (!ParseLanguageFiles(lang_id)) {
		return;
	}
	current_language = lang_id;

	// We reload the entire database as a precaution.
	Player::LoadDatabase();

	// Rewrite our database+messages (unless we are on the Default language).
	// Note that map Message boxes are changed on map load, to avoid slowdown here.
	if (!current_language.empty()) {
		RewriteDatabase();
		RewriteTreemapNames();
		RewriteBattleEventMessages();
		RewriteCommonEventMessages();
	}

	// Reset the cache, so that all images load fresh.
	Cache::Clear();
}


bool Translation::ParseLanguageFiles(const std::string& lang_id)
{
	DirectoryTreeView language_tree;

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
		return true;
	}

	// Scan for files in the directory and parse them.
	for (const auto& tr_name : *language_tree.ListDirectory()) {
		if (tr_name.second.type != DirectoryTree::FileType::Regular) {
			continue;
		}

		auto is = FileFinder::OpenInputStream(language_tree.FindFile(tr_name.first));

		if (tr_name.first == TRFILE_RPG_RT_LDB) {
			sys = std::make_unique<Dictionary>();
			ParsePoFile(std::move(is), *sys);
		} else if (tr_name.first == TRFILE_RPG_RT_BATTLE) {
			battle = std::make_unique<Dictionary>();
			ParsePoFile(std::move(is), *battle);
		} else if (tr_name.first == TRFILE_RPG_RT_COMMON) {
			common = std::make_unique<Dictionary>();
			ParsePoFile(std::move(is), *common);
		} else if (tr_name.first == TRFILE_RPG_RT_LMT) {
			mapnames = std::make_unique<Dictionary>();
			ParsePoFile(std::move(is), *mapnames);
		} else {
			std::unique_ptr<Dictionary> dict;
			dict = std::make_unique<Dictionary>();
			ParsePoFile(std::move(is), *dict);
			maps[tr_name.first] = std::move(dict);
		}
	}

	// Log
	Output::Debug("Translation loaded {} sys, {} common, {} battle, and {} map .po files", (sys==nullptr?0:1), (battle==nullptr?0:1), (common==nullptr?0:1), maps.size());
	return true;
}


void Translation::RewriteDatabase()
{
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

	// Game_Actors caches some values; we need to force re-write them here.
	// As far as I can tell, this is the best way to accomplish this.
	Main_Data::game_actors = std::make_unique<Game_Actors>();
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
		std::string CurrentCmdString() const {
			return ToString(commands[index].string);
		}

		/** Retrieve the indent level of the EventCommand at the index */
		int CurrentCmdIndent() const {
			return commands[index].indent;
		}

		/** Retrieve parameter at position 'pos' of the EventCommand at the current index, or the devValue if no such parameter exists. */
		int CurrentCmdParam(size_t pos, int defVal) const {
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
		void ReWriteString(size_t idx, const std::string& newStr) {
			if (idx < commands.size()) {
				commands[idx].string = lcf::DBString(newStr);
			}
		}

		/**
		 * Puts a "ShowMessage" or "ShowMessage_2" command into the command stack before position "idx".
		 * Sets the string value to "line". Note that ShowMessage_2 is chosen if baseMsgBox is false.
		 * This also updates the index if relevant, but it does not update external index caches.
		 */
		void PutShowMessageBeforeIndex(const std::string& line, size_t idx, bool baseMsgBox) {
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
		for (const std::string& line : lines) {
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
			if (choice_indexes.size()>0) {
				// Translate, break back into lines.
				std::vector<std::vector<std::string>> msgs = TranslateMessageStream(dict, choice_str, '\n');
				if (msgs.size()>0) {
					// Logic here is also based on the last message box.
					std::vector<std::string>& lines = msgs.back();

					// We only pick the first X entries from the translation, since we can't change the Choice count.
					for (size_t num=0; num<choice_indexes.size()&&num<lines.size(); num++) {
						commands.ReWriteString(choice_indexes[num], lines[num]);
					}

					msgs.pop_back();

					// Now add remianing messages, if any
					commands.InsertMultiMessageBefore(msgs, choice_indexes[0]);
				}
			}

			// Note that commands.Advance() has already happened within the above code.
		} else {
			commands.Advance();
		}
	}
}

void Translation::RewriteMapMessages(const std::string& map_name, lcf::rpg::Map& map) {
	// Retrieve lookup for this map.
	auto mapIt = maps.find(map_name);
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
	if (is.good()) {
		if (!Dictionary::FromPo(out, is)) {
			Output::Warning("Failure parsing PO file, resetting");
			out = Dictionary();
		}
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
bool Dictionary::FromPo(Dictionary& res, std::istream& in)
{
	std::string line;
	bool found_header = false;
	bool parse_item = false;

	Entry e;

	auto extract_string = [&line](int offset, bool& error) {
		std::stringstream out;
		bool slash = false;
		bool first_quote = false;

		for (char c : line.substr(offset)) {
			if (c == ' ' && !first_quote) {
				continue;
			} else if (c == '"' && !first_quote) {
				first_quote = true;
				continue;
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
					default:
						Output::Error("Parse error {} ({})", line, c);
						error = true;
						break;
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

		Output::Error("Parse error: Unterminated line: {}", line);
		error = true;
		return out.str();
	};

	auto read_msgstr = [&](bool& error) {
		// Parse multiply lines until empty line or comment
		e.translation = extract_string(6, error);

		while (Utils::ReadLine(in, line)) {
			if (line.empty() || ToStringView(line).starts_with("#")) {
				break;
			}
			e.translation += extract_string(0, error);
		}

		parse_item = false;
		res.addEntry(e);
	};

	auto read_msgid = [&](bool& error) {
		// Parse multiply lines until empty line or msgstr is encountered
		e.original = extract_string(5, error);

		while (Utils::ReadLine(in, line)) {
			if (line.empty() || ToStringView(line).starts_with("msgstr")) {
				read_msgstr(error);
				return;
			}
			e.original += extract_string(0, error);
		}
	};

	bool error = false;
	while (Utils::ReadLine(in, line)) {
		auto lineSV = ToStringView(line);
		if (!found_header) {
			if (lineSV.starts_with("msgstr")) {
				found_header = true;
			}
			continue;
		}

		if (!parse_item) {
			if (lineSV.starts_with("msgctxt")) {
				e.context = extract_string(7, error);
				parse_item = true;
			} else if (lineSV.starts_with("msgid")) {
				parse_item = true;
				read_msgid(error);
			}
		} else {
			if (lineSV.starts_with("msgid")) {
				read_msgid(error);
			} else if (lineSV.starts_with("msgstr")) {
				read_msgstr(error);
			}
		}
	}
	return !error;
}




