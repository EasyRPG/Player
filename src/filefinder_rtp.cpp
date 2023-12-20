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

#include "filefinder_rtp.h"
#include "output.h"
#include "player.h"
#include "registry.h"

#include <lcf/reader_util.h>

#ifdef USE_LIBRETRO
#  include "platform/libretro/ui.h"
#endif

#if defined(USE_SDL) && defined(__ANDROID__)
#  include <jni.h>
#  include <SDL_system.h>
#endif

FileFinder_RTP::FileFinder_RTP(bool no_rtp, bool no_rtp_warnings, std::string rtp_path) {
#ifdef EMSCRIPTEN
	// No RTP support for emscripten at the moment.
	disable_rtp = true;
#else
	disable_rtp = no_rtp;
#endif
	game_has_full_package_flag = no_rtp_warnings;

	if (disable_rtp) {
		Output::Debug("RTP support is disabled.");
		return;
	}

	std::string const version_str =	Player::GetEngineVersion();
	assert(!version_str.empty());

#ifdef __wii__
	AddPath("sd:/data/rtp/" + version_str);
	AddPath("usb:/data/rtp/" + version_str);
#elif defined(__WIIU__)
	AddPath("./rtp/" + version_str);
	AddPath("/data/easyrpg-player/rtp/" + version_str);
#elif defined(__SWITCH__)
	AddPath("./rtp/" + version_str);
	AddPath("/switch/easyrpg-player/rtp/" + version_str);
#elif defined(__3DS__)
	AddPath("romfs:/data/rtp/" + version_str);
	AddPath("sdmc:/data/rtp/" + version_str);
#elif defined(__vita__)
	AddPath("ux0:/data/easyrpg-player/rtp/" + version_str);
#elif defined(__MORPHOS__)
	AddPath("PROGDIR:rtp/" + version_str);
#elif defined(USE_LIBRETRO)
	const char* dir = nullptr;
	if (LibretroUi::environ_cb(RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY, &dir) && dir) {
		AddPath(std::string(dir) + "/rtp/" + version_str);
	}
	if (LibretroUi::environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir) {
		AddPath(std::string(dir) + "/rtp/" + version_str);
	}
#elif defined(__ANDROID__)
	// Invoke "String getRtpPath()" in EasyRPG Activity via JNI
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject sdl_activity = (jobject)SDL_AndroidGetActivity();
	jclass cls = env->GetObjectClass(sdl_activity);
	jmethodID jni_getRtpPath = env->GetMethodID(cls , "getRtpPath", "()Ljava/lang/String;");
	jstring return_string = (jstring)env->CallObjectMethod(sdl_activity, jni_getRtpPath);

	const char *js = env->GetStringUTFChars(return_string, NULL);
	std::string cs(js);

	env->ReleaseStringUTFChars(return_string, js);
	env->DeleteLocalRef(sdl_activity);
	env->DeleteLocalRef(cls);
	AddPath(cs + "/" + version_str);
#elif defined(USE_WINE_REGISTRY) || defined(_WIN32)
	std::string const product = "RPG" + version_str;
	if (Player::IsRPG2k()) {
		// Prefer original 2000 RTP over Kadokawa, because there is no
		// reliable way to detect this engine and much more 2k games
		// use the non-English version
		ReadRegistry("ASCII", product, "RuntimePackagePath");
		ReadRegistry("KADOKAWA", product, "RuntimePackagePath");
	}
	else if (Player::IsRPG2k3E()) {
		// Prefer Kadokawa RTP over Enterbrain for new RPG2k3
		ReadRegistry("KADOKAWA", product, "RuntimePackagePath");
		ReadRegistry("Enterbrain", product, "RUNTIMEPACKAGEPATH");
	}
	else if (Player::IsRPG2k3()) {
		// Original 2003 RTP installer registry key is upper case
		// and Wine registry is case insensitive but new 2k3v1.10 installer is not
		// Prefer Enterbrain RTP over Kadokawa for old RPG2k3 (search order)
		ReadRegistry("Enterbrain", product, "RUNTIMEPACKAGEPATH");
		ReadRegistry("KADOKAWA", product, "RuntimePackagePath");
	}

	// Our RTP is for all engines
	ReadRegistry("EasyRPG", "RTP", "path");
#else
	// Fallback for unknown platforms
	AddPath("/data/rtp/" + version_str);
#endif

#if (defined(PLAYER_NINTENDO) || defined(__vita__) || defined(__ANDROID__))
	// skip environment paths
	return;
#endif

	std::vector<std::string> env_paths;

	// Windows paths are split by semicolon, Unix paths by colon
	std::function<bool(char32_t)> f = [](char32_t t) {
#ifdef _WIN32
		return t == ';';
#else
		return t == ':' || t == ';';
#endif
	};

	if (Player::IsRPG2k() && getenv("RPG2K_RTP_PATH"))
		env_paths = Utils::Tokenize(getenv("RPG2K_RTP_PATH"), f);
	else if (Player::IsRPG2k3() && getenv("RPG2K3_RTP_PATH"))
		env_paths = Utils::Tokenize(getenv("RPG2K3_RTP_PATH"), f);

	if (getenv("RPG_RTP_PATH")) {
		std::vector<std::string> tmp = Utils::Tokenize(getenv("RPG_RTP_PATH"), f);
		env_paths.insert(env_paths.end(), tmp.begin(), tmp.end());
	}

	// If custom RTP paths are set, use these with highest precedence
	if (!rtp_path.empty()) {
		std::vector<std::string> tmp = Utils::Tokenize(rtp_path, f);
		env_paths.insert(env_paths.begin(), tmp.begin(), tmp.end());
	}

#ifdef USE_XDG_RTP
	std::string xdg_rtp;

	// Search in the local data directory
	xdg_rtp = getenv("XDG_DATA_HOME") ? std::string(getenv("XDG_DATA_HOME")) :
			  std::string(getenv("HOME")) + "/.local/share";
	xdg_rtp += "/rtp/" + version_str;
	if (FileFinder::Root().Exists(xdg_rtp)) {
		env_paths.push_back(xdg_rtp);
	}

	// Search in the global data directories
	xdg_rtp = getenv("XDG_DATA_DIRS") ? std::string(getenv("XDG_DATA_DIRS")) :
			  std::string("/usr/local/share/:/usr/share/");
	std::vector<std::string> tmp = Utils::Tokenize(xdg_rtp, f);
	for (StringView p : tmp) {
		xdg_rtp = ToString(p) + (p.back() == '/' ? "" : "/") + "rtp/" + version_str;
		if (FileFinder::Root().Exists(xdg_rtp)) {
			env_paths.push_back(xdg_rtp);
		}
	}
#endif

	// Add all found paths from the environment
	for (StringView p : env_paths) {
		AddPath(p);
	}
}

void FileFinder_RTP::AddPath(StringView p) {
	using namespace FileFinder;
	auto fs = FileFinder::Root().Create(FileFinder::MakeCanonical(p));
	if (fs) {
		auto files = fs.ListDirectory();
		if (files->size() == 0) {
			Output::Debug("RTP path {} is empty, not adding", p);
			return;
		}

		Output::Debug("Adding {} to RTP path", p);

		auto hit_info = RTP::Detect(fs, Player::EngineVersion());

		search_paths.push_back(fs);

		if (hit_info.empty()) {
			Output::Debug("The folder does not contain a known RTP!");
		}

		// Only consider the best RTP hits (usually 100% if properly installed)
		float best = 0.0;
		for (const auto& hit : hit_info) {
			float rate = static_cast<float>(hit.hits) / hit.max;
			if (rate >= best) {
				Output::Debug("RTP is \"{}\" ({}/{})", hit.name, hit.hits, hit.max);
				detected_rtp.emplace_back(hit);
				best = rate;
			}
		}
	} else {
		Output::Debug("RTP path {} is invalid, not adding", p);
	}
}

void FileFinder_RTP::ReadRegistry(StringView company, StringView product, StringView key) {
#if defined(USE_WINE_REGISTRY) || defined(_WIN32)
	std::string rtp_path = Registry::ReadStrValue(
			HKEY_CURRENT_USER, "Software\\" + ToString(company) + "\\" + ToString(product), key, KEY32);
	if (!rtp_path.empty()) {
		AddPath(rtp_path);
	}

	rtp_path = Registry::ReadStrValue(
			HKEY_LOCAL_MACHINE, "Software\\" + ToString(company) + "\\" + ToString(product), key, KEY32);
	if (!rtp_path.empty()) {
		AddPath(rtp_path);
	}
#else
	(void)company;
	(void)product;
	(void)key;
#endif
}

Filesystem_Stream::InputStream FileFinder_RTP::LookupInternal(StringView dir, StringView name, const Span<const StringView> exts, bool& is_rtp_asset) const {
	int version = Player::EngineVersion();

	auto normal_search = [&]() {
		is_rtp_asset = false;
		for (const auto& path : search_paths) {
			const std::string ret = path.FindFile(dir, name, exts);
			if (!ret.empty()) {
				return path.OpenInputStream(ret);
			}
		}
		return Filesystem_Stream::InputStream();
	};

	// Detect the RTP version the game uses, when only one candidate is left the RTP is known
	if (game_rtp.size() != 1) {
		auto candidates = RTP::LookupAnyToRtp(dir, name, version);

		// Prevent Don Miguel RTP addon data from being detected as game RTP because a game can only have one RTP
		// and using this one will break the whole lookup table logic.
		auto addon_it = std::find(candidates.begin(), candidates.end(), RTP::Type::RPG2000_DonMiguelAddon);
		if (addon_it != candidates.end()) {
			candidates.erase(addon_it);
		}

		// when empty the requested asset does not belong to any (known) RTP
		if (!candidates.empty()) {
			if (game_rtp.empty()) {
				game_rtp = candidates;
			} else {
				// Strategy: Remove all RTP that are not candidates by comparing with all previous candidates
				// as the used RTP can only be the one that contains all by now requested assets
				for (auto it = game_rtp.begin(); it != game_rtp.end();) {
					if (std::find(candidates.begin(), candidates.end(), *it) == candidates.end()) {
						it = game_rtp.erase(it);
					} else {
						++it;
					}
				}
			}

			if (game_rtp.size() == 1) {
				// From now on the RTP lookups should be perfect
				Output::Debug("Game uses RTP \"{}\"", RTP::Names[(int) game_rtp[0]]);
			}
		}
	}

	if (game_rtp.empty()) {
		// The game RTP is currently unknown because all requested assets by now were not in any RTP
		// -> fallback to direct search
		is_rtp_asset = false;
		return normal_search();
	}

	// Search across all RTP
	for (const auto& rtp : detected_rtp) {
		for (RTP::Type grtp : game_rtp) {
			std::string rtp_entry = RTP::LookupRtpToRtp(dir, name, grtp, rtp.type, &is_rtp_asset);
			if (!rtp_entry.empty()) {
				std::string ret = rtp.tree.FindFile(dir, rtp_entry, exts);
				if (!ret.empty()) {
					is_rtp_asset = true;
					return rtp.tree.OpenInputStream(ret);
				}
			}
		}
	}

	// Asset is missing or not a RTP asset -> fallback to direct search
	return normal_search();
}

Filesystem_Stream::InputStream FileFinder_RTP::Lookup(StringView dir, StringView name, const Span<const StringView> exts) const {
	if (!disable_rtp) {
		bool is_rtp_asset;
		auto is = LookupInternal(lcf::ReaderUtil::Normalize(dir), lcf::ReaderUtil::Normalize(name), exts, is_rtp_asset);

		std::string lcase = lcf::ReaderUtil::Normalize(dir);
		bool is_audio_asset = lcase == "music" || lcase == "sound";

		if (is_rtp_asset) {
			if (is && game_has_full_package_flag && !warning_broken_rtp_game_shown && !is_audio_asset) {
				warning_broken_rtp_game_shown = true;
				Output::Warning("This game claims it does not need the RTP, but actually uses files from it!");
			} else if (!is && !game_has_full_package_flag && !is_audio_asset) {
				std::string msg = "Cannot find: {}/{}. " +
								  std::string(search_paths.empty() ?
											  "Install RTP {} to resolve this warning." : "RTP {} was probably not installed correctly.");
				Output::Warning(msg, dir, name, Player::EngineVersion());
			}
		}
		return is;
	}

	return Filesystem_Stream::InputStream();
}
