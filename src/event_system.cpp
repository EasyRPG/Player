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

#include "event_system.h"
#include "memory_management.h"
#include "filefinder.h"
#include <map>
#include "output.h"
#include <cstdlib>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

bool FileLoaderAsync::IsReady() const {
	return ready;
}

FileLoaderAsync::FileLoaderAsync(const std::string& folder_name, const std::string& filename, AsyncManager::search_function search_func, void(*call_after_finish)() = NULL, bool critical = false) {
	std::string path = search_func(folder_name, filename);
	this->search_func = search_func;
	this->folder_name = folder_name;
	this->filename = filename;
	this->call_after_finish = call_after_finish;
	this->critical = critical;

	if (!path.empty()) {
		// File is already there but we fake a pending to detect bugs
		fake_wait = true;
		state = 0;
	}
	else {
		fake_wait = false;
#ifdef EMSCRIPTEN
		state = 0;
		std::string p = FileFinder::MakePath(folder_name, filename);
		emscripten_async_wget(("/games/testgame-2000/" + p).c_str(), p.c_str(), AsyncManager::download_success, AsyncManager::download_failure);
#else
		state = 2;
#endif
	}
}

FileLoaderAsync::FileLoaderAsync() {

}

std::string FileLoaderAsync::GetPath() const {
	return search_func(folder_name, filename);
}

void FileLoaderAsync::UpdateProgress() {
	if (fake_wait) {
		if (rand() % 100 == 0) {
			fake_wait = false;
			if (GetPath().empty()) {
				AsyncManager::download_failure(GetName().c_str());
			}
			else {
				AsyncManager::download_success(GetName().c_str());
			}
		}
	}
}

std::string FileLoaderAsync::GetName() const {
	return FileFinder::MakePath(folder_name, filename);
}

namespace {
	std::map<std::string, FileLoaderAsync> pending;
}

FileLoaderAsync AsyncManager::RequestBitmap(const std::string& folder_name, const std::string& filename) {
	std::string path = FileFinder::MakePath(folder_name, filename);

	std::map<std::string, FileLoaderAsync>::iterator it = pending.find(path);

	if (it != pending.end()) {
		it->second.UpdateProgress();
		return it->second;
	}

	pending[path] = FileLoaderAsync(folder_name, filename, &FileFinder::FindImage);

	return pending[path];
}

FileLoaderAsync AsyncManager::RequestFile(const std::string& folder_name, const std::string& filename, void (*call_after_finish)()=NULL, bool critical=false) {
	std::string path = FileFinder::MakePath(folder_name, filename);

	std::map<std::string, FileLoaderAsync>::iterator it = pending.find(path);

	if (it != pending.end()) {
		it->second.UpdateProgress();
		return it->second;
	}

	pending[path] = FileLoaderAsync(folder_name, filename, &FileFinder::FindDefault, call_after_finish, critical);

	return pending[path];
}

void AsyncManager::download_success(const char* filename) {
	std::map<std::string, FileLoaderAsync>::iterator it = pending.find(filename);

	Output::Debug("DL Success %s", it->second.GetName().c_str());

	FileFinder::Init();

	if (it != pending.end()) {
		it->second.state = 1;
		if (it->second.call_after_finish) {
			it->second.call_after_finish();
		}
	}
}

void AsyncManager::download_failure(const char* filename) {
	std::map<std::string, FileLoaderAsync>::iterator it = pending.find(filename);

	Output::Debug("DL Failure %s", it->second.GetName().c_str());

	if (it != pending.end()) {
		it->second.state = 2;
		if (it->second.call_after_finish) {
			it->second.call_after_finish();
		}
	}
}

bool AsyncManager::IsCriticalPending() {
	std::map<std::string, FileLoaderAsync>::iterator it;

	for (it = pending.begin(); it != pending.end(); ++it) {
		it->second.UpdateProgress();

		if (it->second.state == 0 && it->second.critical) {
			Output::Debug("Waiting for %s", it->second.GetName().c_str());
			return true;
		}
	}

	return false;
}

