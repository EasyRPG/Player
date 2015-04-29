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

#include <string>
#include <vector>

class FileLoaderAsync;
class BitmapAsync;
#include "stdint.h"

namespace AsyncManager {
	FileLoaderAsync RequestBitmap(const std::string& folder_name, const std::string& filename);
	FileLoaderAsync RequestFile(const std::string& folder_name, const std::string& filename, void(*call_after_finish)(), bool critical);
	bool IsCriticalPending();
	void Update();

	typedef std::string(*search_function)(const std::string&, const std::string&);

	void download_success(const char* filename);
	void download_failure(const char* filename);
}

	class FileLoaderAsync {
	public:
		FileLoaderAsync();
		FileLoaderAsync(const std::string& folder_name, const std::string& filename, AsyncManager::search_function search_func, void(*call_after_finish)(), bool critical);

		bool IsReady() const;

		void UpdateProgress();

		std::string GetPath() const;
		std::string GetName() const;

		int state;
		bool fake_wait;
		void(*call_after_finish)();
		bool critical;

	private:

		std::string folder_name;
		std::string filename;
		int progress;
		bool ready;
		AsyncManager::search_function search_func;
	};
#endif
