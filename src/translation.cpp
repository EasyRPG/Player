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
#define TRCUST_REMOVEMSG        "__EASY_RPG_CMD:REMOVE_MSGBOX__"
#define TRCUST_ADDMSG           "__EASY_RPG_CMD:ADD_MSGBOX__"


std::string Tr::TranslationDir() {
	return Player::translation.RootDir();
}

std::string Tr::CurrTranslationId() {
	return Player::translation.GetCurrLanguageId();
}

void Translation::Reset()
{
	ClearTranslationLookups();

	languages.clear();
	currLanguage = "";
	translationRootDir = "";
}

void Translation::InitTranslations()
{
	// Reset
	Reset();

	// Determine if the "languages" directory exists, and convert its case.
	auto tree = FileFinder::GetDirectoryTree();
	auto langIt = tree->directories.find(TRDIR_NAME);
	if (langIt != tree->directories.end()) {
		// Save the root directory for later.
		translationRootDir = langIt->second;

		// Now list all directories within the translate dir
		auto translation_path = FileFinder::MakePath(FileFinder::GetDirectoryTree()->directory_path, translationRootDir);
		auto translation_tree = FileFinder::CreateDirectoryTree(translation_path, FileFinder::RECURSIVE);
		if (translation_tree == nullptr) {  return; }

		// Now iterate over every subdirectory.
		for (const auto& trName : translation_tree->directories) {
			Language item;
			item.langDir = trName.second;
			item.langName = trName.second;

			// If there's a manifest file, read the language name and help text from that.
			std::string metaName = FileFinder::FindDefault(*translation_tree, trName.second, TRFILE_META_INI);
			if (!metaName.empty()) {
				lcf::INIReader ini(metaName);
				item.langName = ini.GetString("Language", "Name", item.langName);
				item.langDesc = ini.GetString("Language", "Description", "");
			}

			languages.push_back(item);
		}
	}
}

std::string Translation::GetCurrLanguageId() const 
{
	return currLanguage;
}

std::string Translation::RootDir() const
{
	return translationRootDir;
}

bool Translation::HasTranslations() const 
{
	return !languages.empty();
}

const std::vector<Language>& Translation::GetLanguages() const
{
	return languages;
}


void Translation::SelectLanguage(const std::string& langId)
{
	// Try to read in our language files.
	Output::Debug("Changing language to: '{}'", (!langId.empty() ? langId : "<Default>"));
	if (!ParseLanguageFiles(langId)) {
		return;
	}
	currLanguage = langId;

	// We reload the entire database as a precaution.
	Player::LoadDatabase();

	// Rewrite our database+messages (unless we are on the Default language).
	// Note that map Message boxes are changed on map load, to avoid slowdown here.	
	if (!currLanguage.empty()) {
		RewriteDatabase();
		RewriteTreemapNames();
		RewriteBattleEventMessages();
		RewriteCommonEventMessages();
	}

	// Reset the cache, so that all images load fresh.
	Cache::Clear();
}


bool Translation::ParseLanguageFiles(const std::string& langId)
{
	// Create the directory tree (for lookups).
	std::shared_ptr<FileFinder::DirectoryTree> translation_tree;
	if (langId != "") {
		auto translation_path = FileFinder::MakePath(FileFinder::MakePath(FileFinder::GetDirectoryTree()->directory_path, RootDir()), langId);
		translation_tree = FileFinder::CreateDirectoryTree(translation_path, FileFinder::FILES);
		if (translation_tree == nullptr) {
			Output::Warning("Translation for '{}' does not appear to exist", langId);
			return false;
		}
	}

	// Clear the old translation
	ClearTranslationLookups();

	// For default, this is all we need.
	if (translation_tree == nullptr) {
		return true;
	}

	// Scan for files in the directory and parse them.
	for (const auto& trName : translation_tree->files) {
		std::string fileName = FileFinder::FindDefault(*translation_tree, trName.first);

		if (trName.first == TRFILE_RPG_RT_LDB) {
			sys.reset(new Dictionary());
			ParsePoFile(fileName, *sys);
		} else if (trName.first == TRFILE_RPG_RT_BATTLE) {
			battle.reset(new Dictionary());
			ParsePoFile(fileName, *battle);
		} else if (trName.first == TRFILE_RPG_RT_COMMON) {
			common.reset(new Dictionary());
			ParsePoFile(fileName, *common);
		} else if (trName.first == TRFILE_RPG_RT_LMT) {
			mapnames.reset(new Dictionary());
			ParsePoFile(fileName, *mapnames);
		} else {
			std::unique_ptr<Dictionary> dict;
			dict.reset(new Dictionary());
			ParsePoFile(fileName, *dict);
			maps[trName.first] = std::move(dict);
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

		/// Returns true if the index is past the end of the command list
		bool Done() const {
			return index >= commands.size();
		}

		/// Advance the index through the command list by 1
		void Advance() {
			index += 1;
		}

		/// Retrieve the code of the EventCommand at the index
		lcf::rpg::EventCommand::Code CurrCmdCode() const {
			return static_cast<lcf::rpg::EventCommand::Code>(commands[index].code);
		}

		/// Retrieve the string of the EventCommand at the index
		std::string CurrCmdString() const {
			return ToString(commands[index].string);
		}

		/// Retrieve the indent level of the EventCommand at the index
		int CurrCmdIndent() const {
			return commands[index].indent;
		}

		/// Retrieve parameter at position 'pos' of the EventCommand at the current index, or the devValue if no such parameter exists.
		int CurrCmdParam(size_t pos, int defVal) const {
			if (pos < commands[index].parameters.size()) {
				return commands[index].parameters[pos];
			}
			return defVal;
		}

		/// Returns true if the current Event Command is ShowMessage
		bool CurrIsShowMessage() const {
			return CurrCmdCode() == lcf::rpg::EventCommand::Code::ShowMessage;
		}

		/// Returns true if the current Event Command is ShowMessage_2
		bool CurrIsShowMessage2() const {
			return CurrCmdCode() == lcf::rpg::EventCommand::Code::ShowMessage_2;
		}

		/// Returns true if the current Event Command is ShowChoice
		bool CurrIsShowChoice() const {
			return CurrCmdCode() == lcf::rpg::EventCommand::Code::ShowChoice;
		}

		/// Returns true if the current Event Command is ShowChoiceOption
		bool CurrIsShowChoiceOption() const {
			return CurrCmdCode() == lcf::rpg::EventCommand::Code::ShowChoiceOption;
		}

		/// Returns true if the current Event Command is ShowChoiceEnd
		bool CurrIsShowChoiceEnd() const {
			return CurrCmdCode() == lcf::rpg::EventCommand::Code::ShowChoiceEnd;
		}

		/// Add each line of a [ShowMessage,ShowMessage_2,...] chain to "msg_str" (followed by a newline)
		/// and save to "indexes" the index of each ShowMessage(2) command that was used to populate this
		/// (for rewriting later). 
		/// Advances the index until after the last ShowMessage(2) command
		void BuildMessageString(std::stringstream& msg_str, std::vector<size_t>& indexes) {
			// No change if we're not on the right command.
			if (Done() || !CurrIsShowMessage()) {
				return;
			}

			// Add the first line
			msg_str << CurrCmdString() <<"\n";
			indexes.push_back(index);
			Advance();

			// Build lines 2 through 4
			while (!Done() && CurrIsShowMessage2()) {
				msg_str << CurrCmdString() <<"\n";
				indexes.push_back(index);
				Advance();
			}
		}

		/// Add each line of a [ShowChoice,ShowChoiceOption,...,ShowChoiceEnd] chain to "msg_str" (followed by a newline)
		/// and save to "indexes" the index of each ShowChoiceOption command that was used to populate this
		/// (for rewriting later).
		/// Advances the index until after the (first) ShowChoice command (but it will likely still be on a ShowChoiceOption/End)
		void BuildChoiceString(std::stringstream& msg_str, std::vector<size_t>& indexes) {
			// No change if we're not on the right command.
			if (Done() || !CurrIsShowChoice()) {
				return;
			}

			// Advance to ChoiceOptions
			Advance();
			if(Done()) {
				return;
			}

			// Choices must be on the same indent level.
			// We have to save/restore the index, though, in the rare case that we skip something that can be translated.
			int indent = CurrCmdIndent();
			size_t savedIndex = index;
			while (!Done()) {
				if (indent == CurrCmdIndent()) {
					// Handle a new index
					if (CurrIsShowChoiceOption() && CurrCmdParam(0,0) < 4) {
						msg_str << CurrCmdString() <<"\n";
						indexes.push_back(index);
					}

					// Done?
					if (CurrIsShowChoiceEnd()) {
						break;
					}
				}
				Advance();
			}
			index = savedIndex;
		}

		/// Change the string value of the EventCommand at position "idx" to "newStr"
		void ReWriteString(size_t idx, const std::string& newStr) {
			if (idx < commands.size()) {
				commands[idx].string = lcf::DBString(newStr);
			}
		}

		/// Puts a "ShowMessage" or "ShowMessage_2" command into the command stack before position "idx".
		/// Sets the string value to "line". Note that ShowMessage_2 is chosen if baseMsgBox is false.
		/// This also updates the index if relevant, but it does not update external index caches.
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

		/// Remove the EventCommand at position "idx" from the command stack.
		/// Also updates our index, if relevant.
		void RemoveByIndex(size_t idx) {
			if (idx < commands.size()) {
				commands.erase(commands.begin() + idx);

				// Update our index
				if (index > idx) {
					index -= 1;
				}
			}
		}

		/// Add multiple message boxes to the command stack before "idx".
		/// The "msgs" each represent lines in new, independent message boxes (so they will have both ShowMessage and ShowMessage_2)
		/// Also updates our index, if relevant.
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



std::vector<std::vector<std::string>> Translation::TranslateMessageStream(const Dictionary& dict, const std::stringstream& msg1, const std::stringstream* msg2, char trimChar) {
	// Prepare source string
	std::string msgStr = msg1.str();
	if (msg2!=nullptr) {
		msgStr += msg2->str();
	}
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
		// Logic: build up both the Message stream and the Choice stream, since we'll need to 
		//        deal with both eventually. Then, if they can be merged do that.
		if (commands.CurrIsShowMessage() || commands.CurrIsShowChoice()) {
			// First build up the lines of Message texts
			std::stringstream msg_str;
			std::vector<size_t> msg_indexes;
			commands.BuildMessageString(msg_str, msg_indexes);

			// Next, build up the lines of Choice elements
			std::stringstream choice_str;
			std::vector<size_t> choice_indexes; // Number of entries == number of choices
			commands.BuildChoiceString(choice_str, choice_indexes);

			// Will they fit on screen if we combine them?
			bool combine = false; //msg_indexes.size()>0 && msg_indexes.size()+choice_indexes.size() <= 4;

			// Go through messages first, including possible choices
			if (msg_indexes.size()>0) {
				// Get our lines, possibly including "combined"
				std::vector<std::vector<std::string>> msgs = TranslateMessageStream(dict, msg_str, (combine?&choice_str:nullptr), '\n');
				if (msgs.size()>0) {
					// The complex replacement logic is based on the last message box, then all remaining things are simply left back in.
					std::vector<std::string>& lines = msgs.back();

					// There is a special case here: if we are asked to remove a message box, we should cancel the "combine" action and do nothing further
					// This command is *only* respected as the first line of a message box.
					if (lines[0]==TRCUST_REMOVEMSG) {
						// Update the index of all choice items first
						for (size_t& index : choice_indexes) {
							index -= msg_indexes.size();
						}

						// Now clear all message boxes in reverse order.
						while (!msg_indexes.empty()) {
							commands.RemoveByIndex(msg_indexes.back());
							msg_indexes.pop_back();
						}

						// Finally, reset the "combine" flag so that we process the message box command correctly.
						combine = false;
					} else {
						// We only need the last X Choices from the translation, since we can't change the Choice count.
						size_t maxLines = 4;
						if (combine) {
							// Go backwards through our lines/choices
							while (!choice_indexes.empty() && !lines.empty()) {
								commands.ReWriteString(choice_indexes.back(), lines.back());
								choice_indexes.pop_back();
								lines.pop_back();
								maxLines -= 1;
							}
						}

						// Trim lines down to allowed remaining (with choices).
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

			// Go through choices second.
			if (!combine && choice_indexes.size()>0) {
				// Translate, break back into lines.
				std::vector<std::vector<std::string>> msgs = TranslateMessageStream(dict, choice_str, nullptr, '\n');
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

void Translation::RewriteMapMessages(const std::string& mapName, lcf::rpg::Map& map) {
	// Retrieve lookup for this map.
	auto mapIt = maps.find(mapName);
	if (mapIt==maps.end()) { return; }

	// Rewrite all event commands on all pages.
	for (lcf::rpg::Event& ev : map.events) {
		for (lcf::rpg::EventPage& pg : ev.pages) {
			RewriteEventCommandMessage(*mapIt->second, pg.event_commands);
		}
	}
}


void Translation::ParsePoFile(const std::string& path, Dictionary& out)
{
	std::ifstream in(path.c_str());
	if (in.good()) {
		if (!Dictionary::FromPo(out, in)) {
			Output::Warning("Failure parsing PO file, resetting: '{}'", path);
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

		while (std::getline(in, line, '\n')) {
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

		while (std::getline(in, line, '\n')) {
			if (line.empty() || ToStringView(line).starts_with("msgstr")) {
				read_msgstr(error);
				return;
			}
			e.original += extract_string(0, error);
		}
	};

	bool error = false;
	while (std::getline(in, line, '\n')) {
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




