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

#include <cstdlib>
#include <fstream>
#include <map>

#ifdef EMSCRIPTEN
#  include <emscripten.h>
#  include <lcf/reader_util.h>
#  define PICOJSON_USE_LOCALE 0
#  define PICOJSON_ASSERT(e) do { if (! (e)) assert(false && #e); } while (0)
#  include "external/picojson.h"
#endif

#include "async_handler.h"
#include "cache.h"
#include "filefinder.h"
#include "memory_management.h"
#include "output.h"
#include "player.h"
#include "main_data.h"
#include "utils.h"
#include "transition.h"
#include "rand.h"

// When this option is enabled async requests are randomly delayed.
// This allows testing some aspects of async file fetching locally.
//#define EP_DEBUG_SIMULATE_ASYNC

namespace {
	std::unordered_map<std::string, FileRequestAsync> async_requests;
	std::unordered_map<std::string, std::string> file_mapping;
	int next_id = 0;
#ifdef EMSCRIPTEN
	int index_version = 1;
#endif

	FileRequestAsync* GetRequest(const std::string& path) {
		auto it = async_requests.find(path);

		if (it != async_requests.end()) {
			return &(it->second);
		}
		return nullptr;
	}

	FileRequestAsync* RegisterRequest(std::string path, std::string directory, std::string file)
	{
		auto req = FileRequestAsync(path, std::move(directory), std::move(file));
		auto p = async_requests.emplace(std::make_pair(std::move(path), std::move(req)));
		return &p.first->second;
	}

	FileRequestBinding CreatePending() {
		return std::make_shared<int>(next_id++);
	}

#ifdef EMSCRIPTEN
	constexpr size_t ASYNC_MAX_RETRY_COUNT{ 16 };

	struct async_download_context {
		std::string url, file, param;
		FileRequestAsync* obj;
		size_t count;

		async_download_context(
			std::string u,
			std::string f,
			std::string p,
			FileRequestAsync* o
		) : url{ std::move(u) }, file{ std::move(f) }, param{ std::move(p) }, obj{ o }, count{} {}
	};

	void download_success_retry(unsigned, void* userData, const char*) {
		auto ctx = static_cast<async_download_context*>(userData);
		ctx->obj->DownloadDone(true);
		delete ctx;
	}

	void start_async_wget_with_retry(async_download_context* ctx);

	void download_failure_retry(unsigned, void* userData, int status) {
		auto ctx = static_cast<async_download_context*>(userData);
		++ctx->count;
		if (ctx->count >= ASYNC_MAX_RETRY_COUNT) {
			Output::Warning("DL Failure: max retries exceeded: {}", ctx->obj->GetPath());
			ctx->obj->DownloadDone(false);
			delete ctx;
			return;
		}
		if (status >= 400) {
			Output::Warning("DL Failure: file not available: {}", ctx->obj->GetPath());
			ctx->obj->DownloadDone(false);
			delete ctx;
			return;
		}
		Output::Debug("DL Failure: {}. Retrying", ctx->obj->GetPath());
		start_async_wget_with_retry(ctx);
	}

	void start_async_wget_with_retry(async_download_context* ctx) {
		emscripten_async_wget2(
			ctx->url.data(),
			ctx->file.data(),
			"GET",
			ctx->param.data(),
			ctx,
			download_success_retry,
			download_failure_retry,
			nullptr
		);
	}

	void async_wget_with_retry(
		std::string url,
		std::string file,
		std::string param,
		FileRequestAsync* obj
	) {
		// ctx will be deleted when download succeeds
		auto ctx = new async_download_context{ url, file, param, obj };
		start_async_wget_with_retry(ctx);
	}

#endif
}

void AsyncHandler::CreateRequestMapping(const std::string& file) {
#ifdef EMSCRIPTEN
	auto f = FileFinder::Game().OpenInputStream(file);
	if (!f) {
		Output::Error("Emscripten: Reading index.json failed");
		return;
	}

	picojson::value v;
	picojson::parse(v, f);

	const auto& metadata = v.get("metadata");
	if (metadata.is<picojson::object>()) {
		for (const auto& value : metadata.get<picojson::object>()) {
			if (value.first == "version") {
				index_version = (int)value.second.get<double>();
			}
		}
	}

	Output::Debug("Parsing index.json version {}", index_version);

	if (index_version <= 1) {
		// legacy format
		for (const auto& value : v.get<picojson::object>()) {
			file_mapping[value.first] = value.second.to_str();
		}
	} else {
		using fn = std::function<void(const picojson::object&,const std::string&)>;
		fn parse = [&] (const picojson::object& obj, const std::string& path) {
			std::string dirname;
			for (const auto& value : obj) {
				if (value.first == "_dirname" && value.second.is<std::string>()) {
					dirname = value.second.to_str();
				}
			}
			dirname = FileFinder::MakePath(path, dirname);

			for (const auto& value : obj) {
				const auto& second = value.second;
				if (second.is<picojson::object>()) {
					parse(second.get<picojson::object>(), dirname);
				} else if (second.is<std::string>()){
					file_mapping[FileFinder::MakePath(Utils::LowerCase(dirname), value.first)] = FileFinder::MakePath(dirname, second.to_str());
				}
			}
		};

		const auto& cache = v.get("cache");
		if (cache.is<picojson::object>()) {
			parse(cache.get<picojson::object>(), "");
		}

		// Create some empty DLL files. Engine & patch detection depend on them.
		for (const auto& s : {"harmony.dll", "ultimate_rt_eb.dll", "dynloader.dll", "accord.dll"}) {
			auto it = file_mapping.find(s);
			if (it != file_mapping.end()) {
				FileFinder::Game().OpenOutputStream(s);
			}
		}

		// Look for Meta.ini files and fetch them. They are required for detecting the translations.
		for (const auto& item: file_mapping) {
			if (StringView(item.first).ends_with("meta.ini")) {
				auto* request = AsyncHandler::RequestFile(item.second);
				request->SetImportantFile(true);
				request->Start();
			}
		}
	}
#else
	// no-op
	(void)file;
#endif
}

void AsyncHandler::ClearRequests() {
	async_requests.clear();
}

FileRequestAsync* AsyncHandler::RequestFile(StringView folder_name, StringView file_name) {
	auto path = FileFinder::MakePath(folder_name, file_name);

	auto* request = GetRequest(path);

	if (request) {
		return request;
	}

	//Output::Debug("Waiting for {}", path);

	return RegisterRequest(std::move(path), std::string(folder_name), std::string(file_name));
}

FileRequestAsync* AsyncHandler::RequestFile(StringView file_name) {
	return RequestFile(".", file_name);
}

bool AsyncHandler::IsFilePending(bool important, bool graphic) {
	for (auto& ap: async_requests) {
		FileRequestAsync& request = ap.second;

#ifdef EP_DEBUG_SIMULATE_ASYNC
		request.UpdateProgress();
#endif

		if (!request.IsReady()
				&& (!important || request.IsImportantFile())
				&& (!graphic || request.IsGraphicFile())
				) {
			return true;
		}
	}

	return false;
}

bool AsyncHandler::IsImportantFilePending() {
	return IsFilePending(true, false);
}

bool AsyncHandler::IsGraphicFilePending() {
	return IsFilePending(false, true);
}

FileRequestAsync::FileRequestAsync(std::string path, std::string directory, std::string file) :
	directory(std::move(directory)),
	file(std::move(file)),
	path(std::move(path)),
	state(State_WaitForStart)
{ }

void FileRequestAsync::SetGraphicFile(bool graphic) {
	this->graphic = graphic;
	// We need this flag in order to prevent show screen transitions
	// from starting util all graphical assets are loaded.
	// Also, the screen is erased, so you can't see any delays :)
	if (Transition::instance().IsErasedNotActive()) {
		SetImportantFile(true);
	}
}

void FileRequestAsync::Start() {
	if (file == CACHE_DEFAULT_BITMAP) {
		// Embedded asset -> Fire immediately
		DownloadDone(true);
		return;
	}

	if (state == State_Pending) {
		return;
	}

	if (IsReady()) {
		// Fire immediately
		DownloadDone(true);
		return;
	}

	state = State_Pending;

#ifdef EMSCRIPTEN
	std::string request_path;
#  ifdef EM_GAME_URL
	request_path = EM_GAME_URL;
#  else
	request_path = "games/";
#  endif

	if (!Player::emscripten_game_name.empty()) {
		request_path += Player::emscripten_game_name + "/";
	} else {
		request_path += "default/";
	}

	std::string modified_path;
	if (index_version >= 2) {
		modified_path = lcf::ReaderUtil::Normalize(path);
		modified_path = FileFinder::MakeCanonical(modified_path, 1);
	} else {
		modified_path = Utils::LowerCase(path);
		if (directory != ".") {
			modified_path = FileFinder::MakeCanonical(modified_path, 1);
		} else {
			auto it = file_mapping.find(modified_path);
			if (it == file_mapping.end()) {
				modified_path = FileFinder::MakeCanonical(modified_path, 1);
			}
		}
	}

	if (graphic && Tr::HasActiveTranslation()) {
		std::string modified_path_trans = FileFinder::MakePath(lcf::ReaderUtil::Normalize(Tr::GetCurrentTranslationFilesystem().GetFullPath()), modified_path);
		auto it = file_mapping.find(modified_path_trans);
		if (it != file_mapping.end()) {
			modified_path = modified_path_trans;
		}
	}

	auto it = file_mapping.find(modified_path);
	if (it != file_mapping.end()) {
		request_path += it->second;
	} else {
		if (file_mapping.empty()) {
			// index.json not fetched yet, fallthrough and fetch
			request_path += path;
		} else {
			// Fire immediately (error)
			Output::Debug("{} not in index.json", modified_path);
			DownloadDone(false);
			return;
		}
	}

	// URL encode %, # and +
	request_path = Utils::ReplaceAll(request_path, "%", "%25");
	request_path = Utils::ReplaceAll(request_path, "#", "%23");
	request_path = Utils::ReplaceAll(request_path, "+", "%2B");

	auto request_file = (it != file_mapping.end() ? it->second : path);
	async_wget_with_retry(request_path, std::move(request_file), "", this);
#else
#  ifdef EM_GAME_URL
#    warning EM_GAME_URL set and not an Emscripten build!
#  endif

#  ifndef EP_DEBUG_SIMULATE_ASYNC
	DownloadDone(true);
#  endif
#endif
}

void FileRequestAsync::UpdateProgress() {
#ifndef EMSCRIPTEN
	// Fake download for testing event handlers

	if (!IsReady() && Rand::ChanceOf(1, 100)) {
		DownloadDone(true);
	}
#endif
}

FileRequestBinding FileRequestAsync::Bind(void(*func)(FileRequestResult*)) {
	FileRequestBinding pending = CreatePending();

	listeners.emplace_back(FileRequestBindingWeak(pending), func);

	return pending;
}

FileRequestBinding FileRequestAsync::Bind(std::function<void(FileRequestResult*)> func) {
	FileRequestBinding pending = CreatePending();

	listeners.emplace_back(FileRequestBindingWeak(pending), func);

	return pending;
}

void FileRequestAsync::CallListeners(bool success) {
	FileRequestResult result { directory, file, -1, success };

	for (auto& listener : listeners) {
		if (!listener.first.expired()) {
			result.request_id = *listener.first.lock();
			(listener.second)(&result);
		} else {
			Output::Debug("Request cancelled: {}", GetPath());
		}
	}

	listeners.clear();
}

void FileRequestAsync::DownloadDone(bool success) {
	if (IsReady()) {
		// Change to real success state when already finished before
		success = state == State_DoneSuccess;
	}

	if (success) {
#ifdef EMSCRIPTEN
		if (state == State_Pending) {
			// Update directory structure (new file was added)
			if (FileFinder::Game()) {
				FileFinder::Game().ClearCache();
			}
		}
#endif

		state = State_DoneSuccess;

		CallListeners(true);
	}
	else {
		state = State_DoneFailure;

		CallListeners(false);
	}
}
