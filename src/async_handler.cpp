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

#include "async_handler.h"
#include "memory_management.h"
#include "filefinder.h"
#include <map>
#include "output.h"
#include <cstdlib>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

namespace {
	std::map<std::string, FileLoaderAsync> async_requests;

	FileLoaderAsync* GetRequest(const std::string& path) {
		std::map<std::string, FileLoaderAsync>::iterator it = async_requests.find(path);

		if (it != async_requests.end()) {
			return &(it->second);
		}

		return NULL;
	}

	void RegisterRequest(const std::string& path, const FileLoaderAsync& request) {
		async_requests[path] = request;
	}
}

FileLoaderAsync* AsyncHandler::RequestFile(const std::string& folder_name, const std::string& file_name) {
	std::string path = FileFinder::MakePath(folder_name, file_name);

	FileLoaderAsync* request = GetRequest(path);

	if (request) {
		return request;
	}

	FileLoaderAsync req(path);
	RegisterRequest(path, req);

	Output::Debug("Waiting for %s", path.c_str());

	return GetRequest(path);
}

bool AsyncHandler::IsImportantFilePending() {
	std::map<std::string, FileLoaderAsync>::iterator it;

	for (it = async_requests.begin(); it != async_requests.end(); ++it) {
		it->second.UpdateProgress();

		if (!it->second.IsReady() && it->second.IsImportantFile()) {
			Output::Debug("Waiting for %s", it->second.GetPath().c_str());
			return true;
		}
	}

	return false;
}

FileLoaderAsync::FileLoaderAsync(const std::string& path) {
	this->path = path;
	this->important = false;

	state = 0;
}

FileLoaderAsync::FileLoaderAsync() {
}

bool FileLoaderAsync::IsReady() const {
	return state != 0;
}

bool FileLoaderAsync::IsImportantFile() const {
	return important;
}

void FileLoaderAsync::SetImportantFile(bool important) {
	this->important = important;
}

void FileLoaderAsync::Start() {
	if (IsReady()) {
		// Fire immediately
		DownloadSuccess(NULL);
		DownloadFailure(NULL);
	}

#ifdef EMSCRIPTEN
	boost::function1<void, const char*> success, failure;
	success = std::bind1st(std::mem_fun(&FileLoaderAsync::DownloadSuccess), this);
	failure = std::bind1st(std::mem_fun(&FileLoaderAsync::DownloadFailure), this);

	emscripten_async_wget(("/games/testgame-2000/" + path).c_str(), path.c_str(), success, failure);
#endif
}

void FileLoaderAsync::UpdateProgress() {
#ifndef EMSCRIPTEN
	// Fake download for testing event handlers

	if (!IsReady() && rand() % 100 == 0) {
		DownloadSuccess(path.c_str());
	}
#endif
}

const std::string& FileLoaderAsync::GetPath() const {
	return path;
}

void FileLoaderAsync::Bind(void(*func)(int)) {
	listeners.push_back(func);
}

void FileLoaderAsync::CallListeners(bool success) {
	std::vector<boost::function<void(bool)> >::iterator it;
	for (it = listeners.begin(); it != listeners.end(); ++it) {
		(*it)(success);
	}

	listeners.clear();
}

void FileLoaderAsync::DownloadSuccess(const char*) {
	Output::Debug("DL Success %s", path.c_str());

#ifdef EMSCRIPTEN
	FileFinder::Init();
#endif

	CallListeners(true);

	state = 1;
}

void FileLoaderAsync::DownloadFailure(const char*) {
	Output::Debug("DL Failure %s", path.c_str());

	CallListeners(false);

	state = 2;
}
