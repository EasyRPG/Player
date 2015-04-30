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

#ifndef _EASYRPG_ASYNC_MANAGER_H_
#define _EASYRPG_ASYNC_MANAGER_H_

#include <boost/function.hpp>
#include <string>
#include <vector>

class FileRequestAsync;

namespace AsyncHandler {
	FileRequestAsync* RequestFile(const std::string& folder_name, const std::string& filename);
	FileRequestAsync* RequestFile(const std::string& filename);

	bool IsImportantFilePending();
	void Update();
}

class FileRequestAsync {
public:
	FileRequestAsync();
	FileRequestAsync(const std::string& path);

	bool IsReady() const;
	bool IsImportantFile() const;
	void SetImportantFile(bool important);

	void Start();

	void UpdateProgress();

	const std::string& GetPath() const;

	std::vector<boost::function<void(bool)> > listeners;
	template<typename T> void Bind(void (T::*func)(bool), T* that);

	void Bind(boost::function<void(bool)> func);
	void Bind(void(*func)(bool));

private:
	void CallListeners(bool success);

	void DownloadSuccess(const char* filename);
	void DownloadFailure(const char* filename);

	std::string path;
	int state;
	bool important;
};

template<typename T>
void FileRequestAsync::Bind(void (T::*func)(bool), T* that) {
	boost::function1<void, bool> f;
	f = std::bind1st(std::mem_fun(func), that);
	listeners.push_back(f);
}
#endif
