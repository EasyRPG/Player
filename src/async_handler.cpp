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
#include "async_handler.h"
#include "filefinder.h"
#include "memory_management.h"
#include "output.h"
#include "player.h"

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

namespace {
	std::map<std::string, FileRequestAsync> async_requests;
	int next_id = 0;

	FileRequestAsync* GetRequest(const std::string& path) {
		std::map<std::string, FileRequestAsync>::iterator it = async_requests.find(path);

		if (it != async_requests.end()) {
			return &(it->second);
		}
		return NULL;
	}

	void RegisterRequest(const std::string& path, const FileRequestAsync& request) {
		async_requests[path] = request;
	}

	void download_success(unsigned, void* userData, const char*) {
		(static_cast<FileRequestAsync*>(userData))->DownloadDone(true);
	}

	void download_failure(unsigned, void* userData, int) {
		(static_cast<FileRequestAsync*>(userData))->DownloadDone(false);
	}
}

FileRequestAsync* AsyncHandler::RequestFile(const std::string& folder_name, const std::string& file_name) {
	std::string path = FileFinder::MakePath(folder_name, file_name);

	FileRequestAsync* request = GetRequest(path);

	if (request) {
		return request;
	}

	FileRequestAsync req(folder_name, file_name);
	RegisterRequest(path, req);

	//Output::Debug("Waiting for %s", path.c_str());

	return GetRequest(path);
}

FileRequestAsync* AsyncHandler::RequestFile(const std::string& file_name) {
	return RequestFile(".", file_name);
}

bool AsyncHandler::IsImportantFilePending() {
	std::map<std::string, FileRequestAsync>::iterator it;

	for (it = async_requests.begin(); it != async_requests.end(); ++it) {
		FileRequestAsync& request = it->second;
		// remove comment for fake download testing
		//request.UpdateProgress();

		if (!request.IsReady() && request.IsImportantFile()) {
			return true;
		}
	}

	return false;
}

FileRequestAsync::FileRequestAsync(const std::string& folder_name, const std::string& file_name) :
	directory(folder_name),
	file(file_name) {
	this->path = path = FileFinder::MakePath(folder_name, file_name);
	this->important = false;

	state = State_WaitForStart;
}

FileRequestAsync::FileRequestAsync() {
}

bool FileRequestAsync::IsReady() const {
	return state == State_DoneSuccess || state == State_DoneFailure;
}

bool FileRequestAsync::IsImportantFile() const {
	return important;
}

void FileRequestAsync::SetImportantFile(bool important) {
	this->important = important;
}

void FileRequestAsync::Start() {
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
	std::string request_path = "games/?file=" + path;
	if (!Player::emscripten_game_name.empty()) {
		request_path += "&game=" + Player::emscripten_game_name;
	}

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
	// add comment for fake download testing
	DownloadDone(true);
#endif
}

void FileRequestAsync::UpdateProgress() {
#ifndef EMSCRIPTEN
	// Fake download for testing event handlers

	if (!IsReady() && rand() % 100 == 0) {
		DownloadDone(true);
	}
#endif
}

const std::string& FileRequestAsync::GetPath() const {
	return path;
}

int FileRequestAsync::Bind(void(*func)(FileRequestResult*)) {
	listeners.push_back(std::make_pair(next_id, func));

	return next_id++;
}

int FileRequestAsync::Bind(boost::function<void(FileRequestResult*)> func) {
	listeners.push_back(std::make_pair(next_id, func));

	return next_id++;
}

bool FileRequestAsync::Unbind(int id) {
	std::vector<std::pair<int, boost::function<void(FileRequestResult*)> > >::iterator it;

	for (it = listeners.begin(); it != listeners.end(); ++it) {
		if (it->first == id) {
			listeners.erase(it);
			return true;
		}
	}

	return false;
}

void FileRequestAsync::CallListeners(bool success) {
	FileRequestResult result;
	result.directory = directory;
	result.file = file;
	result.success = success;

	std::vector<std::pair<int, boost::function<void(FileRequestResult*)> > >::iterator it;
	for (it = listeners.begin(); it != listeners.end(); ++it) {
		(it->second)(&result);
	}

	listeners.clear();
}

void FileRequestAsync::DownloadDone(bool success) {
	if (IsReady()) {
		// Change to real success state when already finished before
		success = state == State_DoneSuccess;
	}

	if (success) {
		//Output::Debug("DL Success %s", path.c_str());

#ifdef EMSCRIPTEN
		if (state == State_Pending) {
			FileFinder::Init();
		}
#endif

		state = State_DoneSuccess;

		CallListeners(true);
	}
	else {
		//Output::Debug("DL Failure %s", path.c_str());

		state = State_DoneFailure;

		CallListeners(false);
	}
}
