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
#include <map>

#ifdef EMSCRIPTEN
#  include <emscripten.h>
#  include <regex>
#  include "picojson.h"
#endif

#include "async_handler.h"
#include "cache.h"
#include "filefinder.h"
#include "memory_management.h"
#include "output.h"
#include "player.h"
#include "main_data.h"
#include <fstream>
#include "utils.h"
#include "transition.h"

// When this option is enabled async requests are randomly delayed.
// This allows testing some aspects of async file fetching locally.
//#define EP_DEBUG_SIMULATE_ASYNC

namespace {
	std::unordered_map<std::string, FileRequestAsync> async_requests;
	std::unordered_map<std::string, std::string> file_mapping;
	int next_id = 0;

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
	void download_success(unsigned, void* userData, const char*) {
		FileRequestAsync* req = static_cast<FileRequestAsync*>(userData);
		//Output::Debug("DL Success: {}", req->GetPath());
		req->DownloadDone(true);
	}

	void download_failure(unsigned, void* userData, int) {
		FileRequestAsync* req = static_cast<FileRequestAsync*>(userData);
		Output::Debug("DL Failure: {}", req->GetPath());
		req->DownloadDone(false);
	}
#endif
}

void AsyncHandler::CreateRequestMapping(const std::string& file) {
#ifdef EMSCRIPTEN
	std::shared_ptr<std::fstream> f = FileFinder::openUTF8(file, std::ios_base::in | std::ios_base::binary);
	picojson::value v;
	picojson::parse(v, *f);

	for (const auto& value : v.get<picojson::object>()) {
		file_mapping[value.first] = value.second.to_str();
	}
#else
	// no-op
	(void)file;
#endif
}

FileRequestAsync* AsyncHandler::RequestFile(const std::string& folder_name, const std::string& file_name) {
	auto path = FileFinder::MakePath(folder_name, file_name);

	auto* request = GetRequest(path);

	if (request) {
		return request;
	}

	//Output::Debug("Waiting for {}", path);

	return RegisterRequest(std::move(path), folder_name, file_name);
}

FileRequestAsync* AsyncHandler::RequestFile(const std::string& file_name) {
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

	std::string real_path = Utils::LowerCase(path);
	if (directory != ".") {
		// Don't alter the path when the file is in the main directory
		real_path = FileFinder::MakeCanonical(real_path, 1);
	}

	auto it = file_mapping.find(real_path);
	if (it != file_mapping.end()) {
		request_path += it->second;
	} else {
		// Fall through if not found, will fail in the ajax request
		request_path += path;
	}

	// URL encode % and #
	request_path = std::regex_replace(request_path, std::regex("%"), "%25");
	request_path = std::regex_replace(request_path, std::regex("#"), "%23");

	emscripten_async_wget2(
		request_path.c_str(),
		path.c_str(),
		"GET",
		NULL,
		this,
		download_success,
		download_failure,
		NULL);
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

	if (!IsReady() && Utils::ChanceOf(1, 100)) {
		DownloadDone(true);
	}
#endif
}

FileRequestBinding FileRequestAsync::Bind(void(*func)(FileRequestResult*)) {
	FileRequestBinding pending = CreatePending();

	listeners.push_back(std::make_pair(FileRequestBindingWeak(pending), func));

	return pending;
}

FileRequestBinding FileRequestAsync::Bind(std::function<void(FileRequestResult*)> func) {
	FileRequestBinding pending = CreatePending();

	listeners.push_back(std::make_pair(FileRequestBindingWeak(pending), func));

	return pending;
}

void FileRequestAsync::CallListeners(bool success) {
	FileRequestResult result { directory, file, success };

	for (auto& listener : listeners) {
		if (!listener.first.expired()) {
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
			FileFinder::SetDirectoryTree(FileFinder::CreateDirectoryTree(Main_Data::GetProjectPath()));
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
