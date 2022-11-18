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

#ifndef EP_TRANSLATION_H
#define EP_TRANSLATION_H

// Headers
#include <string>
#include <sstream>
#include <memory>
#include <unordered_map>

#include "async_handler.h"
#include "filefinder.h"

namespace lcf {
	namespace rpg {
		class Map;
		class EventCommand;
	}
	class DBString;
}


/**
 * Namespace used to retrieve translations of Terms, BattlEvents, etc.
 */
namespace Tr {
	/**
	 * @return The directory tree of the translation directory.
	 */
	FilesystemView GetTranslationFilesystem();

	/**
	 * @return Whether a translation is active.
	 */
	bool HasActiveTranslation();

	/**
	 * The id of the current translation (e.g., "Spanish"). If empty, there is no active translation.
	 * @return The translation ID
	 */
	std::string GetCurrentTranslationId();

	/**
	 * @return Language code of the current active translation. If empty none was set or no active translation.
	 */
	std::string GetCurrentLanguageCode();

	/**
	 * @return The directory tree of the active translation.
	 */
	FilesystemView GetCurrentTranslationFilesystem();

} // End namespace Tr


//////////////////////////////////////////////////////////
// NOTE: The code for Entry and Dictionary is duplicated in LcfTrans.
//       At some point it should be merged to a common location.
//////////////////////////////////////////////////////////

/**
 * An entry in the dictionary
 */
class Entry {
public:
	std::string original; // msgid
	std::string translation; // msgstr
	std::string context; // msgctxt
};

/**
 * A .po file loaded into memory. Contains a dictionary of entries.
 */
class Dictionary {
public:
	/**
	 * Super simple parser.
	 * Only parses msgstr, msgid and msgctx
	 * Program aborts on error
	 *
	 * @param res The dictionary to store the translated entries in.
	 * @param in The stream to load the translated entries from.
	 */
	static void FromPo(Dictionary& res, std::istream& in);

	/**
	 * Replace an original string with the translated string.
	 * Template can be "std::string" or "lcf::DBString"
	 *
	 * @param context The 'context' of this string; used to differentiate strings that have the same
	 *                original language value but different target language values.
	 * @param original The string to lookup. Will be replaced if a lookup is found.
	 * @return True if the original string was replaced; false otherwise.
	 */
	template <class StringType>
	bool TranslateString(StringView context, StringType& original) const;

private:
	/**
	 * Add an entry to the dictionary.
	 *
	 * @param entry The entry to add.
	 */
	void addEntry(const Entry& entry);

	// Lookup by context, where context can be empty ("") for no context.
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> entries;
};


// Template implementation
template <class StringType>
bool Dictionary::TranslateString(StringView context, StringType& original) const
{
	std::stringstream key;
	key << original;
	auto it = entries.find(ToString(context));
	if (it != entries.end()) {
		auto it2 = it->second.find(key.str());
		if (it2 != it->second.end()) {
			original = StringType(it2->second);
			return true;
		}
	}
	return false;
}


/**
 * Properties of a language
 */
struct Language {
	std::string lang_dir;  // Language directory (e.g., "en", "English_Localization")
	std::string lang_name; // Display name for this language (e.g., "English")
	std::string lang_desc; // Helper text to show when the menu is highlighted
	std::string lang_code; // Language code used by font selection and input scene
	std::string lang_term; // Term to use for "Language"
	bool use_builtin_font = false;
};

/**
 * Class that holds a list of all available translations and tracks the current one.
 * Allows changing the language and rewriting the database.
 */
class Translation {
public:
	/**
	 * Scan the Languages directory and build up a list of known languages.
	 */
	void InitTranslations();

	/**
	 * Do any translations besides the default exist?
	 *
	 * @return True if there is at least one translation (language); false otherwise
	 */
	bool HasTranslations() const;

	/**
	 * Retrieve the root directory tree for all Languages
	 *
	 * @return the Languages directory tree.
	 */
	FilesystemView GetRootTree() const;

	/**
	 * Retrieves a vector of all known languages.
	 *
	 * @return the Languages vector
	 */
	const std::vector<Language>& GetLanguages() const;

	/**
	 * Switches to a given language. Resets the database and the Image Cache.
	 *
	 * @param lang_id The language ID (or "" for "Default")
	 */
	void SelectLanguage(StringView lang_id);

	/**
	 * Does a async fetch of a map po file.
	 * Only used by the web player.
	 *
	 * @param map_id map whose po file to fetch
	 */
	void RequestAndAddMap(int map_id);

	/**
	 * Rewrite all Messages and Choices in this Map
	 *
	 * @param map_name The name of the map with formatting similar to the .po file; e.g., "map0104.po"
	 * @param map The map object itself (for modifying).
	 */
	void RewriteMapMessages(StringView map_name, lcf::rpg::Map& map);

	/**
	 * Retrieve the current language.
	 *
	 * @return the current language
	 */
	const Language& GetCurrentLanguage() const;

	/**
	 * Retrieves information about the builtin language of the game.
	 * This is obtained from Meta.ini in the "default" directory.
	 *
	 * @return the default language
	 */
	const Language& GetDefaultLanguage() const;


private:
	void SelectLanguageAsync(FileRequestResult* result, StringView lang_id);

	/**
	 * Reset all saved language data and revert to "no translation".
	 */
	void Reset();

	/**
	 * Reset all lookups loaded from .po files for the active language.
	 */
	void ClearTranslationLookups();

	/**
	 * Parse a .po file and save its language-related strings.
	 *
	 * @param is Handle to a Po file to read.
	 * @param out The Dictionary to save these entries in (output).
	 */
	void ParsePoFile(Filesystem_Stream::InputStream is, Dictionary& out);

	/**
	 * Rewrite RPG_RT.ldb with the current translation entries
	 */
	void RewriteDatabase();

	/**
	 * Rewrite RPG_RT.lmt with the current translation entries
	 */
	void RewriteTreemapNames();

	/**
	 * Rewrite all Battle Messages with the current translation entries
	 */
	void RewriteBattleEventMessages();

	/**
	 * Rewrite all Common Event Messages with the current translation entries
	 */
	void RewriteCommonEventMessages();

	/**
	 * Convert a stream of msgbox or choices to a list of output message boxes
	 *
	 * @param dict The dictionary to use for translation
	 * @param msg The message string to use for lookup. String with newlines.
	 * @param trimChar Trim this character if the lookup string ends with this.
	 * @return A vector of Message Boxes, where each Message Box is represented as a vector of lines (strings), or an empty vector if there is no translation.
	 *         It is guaranteed that each MessageBox vector will have at least one entry (containing "") if it would otherwise be empty; this can happen
	 *         if the message box insertion commands are used. Note that the last MessageBox vector may contain translated "Choice" entries (it is based on the input).
	 */
	std::vector<std::vector<std::string>> TranslateMessageStream(const Dictionary& dict, const std::stringstream& msg, char trimChar);

	/**
	 * Rewrite a list of event commands (from any map, battle, or common event) given a dictionary.
	 * Takes into account deleting and adding message boxes.
	 *
	 * @param dict The dictionary to use for translation.
	 * @param commands The commands to search through and update.
	 */
	void RewriteEventCommandMessage(const Dictionary& dict, std::vector<lcf::rpg::EventCommand>& commands);


private:
	/**
	 * Parse all .po files for the given language.
	 *
	 * @param lang_id The ID of the language to parse, or "" for Default (no parsing is done)
	 * @return True if the language directory was found; false otherwise
	 */
	bool ParseLanguageFiles(StringView lang_id);

	// Our translations are broken apart into multiple files; we store a lookup for each one.
	std::unique_ptr<Dictionary> sys;       // RPG_RT.ldb.po
	std::unique_ptr<Dictionary> common;    // RPG_RT.ldb.common.po
	std::unique_ptr<Dictionary> battle;    // RPG_RT.ldb.battle.po
	std::unique_ptr<Dictionary> mapnames;  // RPG_RT.lmt.po (map names, used only in the "Teleport" event command)
	std::unordered_map<std::string, std::unique_ptr<Dictionary>> maps;  // map<id>.po, indexed by map name

	// Our list of available Languages (translations, localizations), determined by scanning the files on disk.
	std::vector<Language> languages;

	// The "languages" directory, but with appropriate capitalization.
	FilesystemView translation_root_fs;

	// The translation we are currently showing (e.g., "English_1")
	Language current_language;

	// The normal language of the game
	// Provides data from the Meta.ini in "default" folder when available
	Language default_language;

	std::vector<FileRequestBinding> requests;
	FileRequestBinding map_request;
	int request_counter = -1;
};

#endif  // EP_TRANSLATION_H
