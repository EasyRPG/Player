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
struct FileRequestResult;

/**
 * AsyncHandler supports asynchronous file requests for platforms that don't
 * support synchronous IO (e.g. Emscripten).
 */
namespace AsyncHandler {
	/**
	 * Creates a request to a file.
	 * When the same file was already requested this will return an already
	 * existing request.
	 * The caller shouldn't choose different code path depending on new or
	 * existing requests.
	 *
	 * @param folder_name folder where the file is stored
	 * @param file_name Name of the requested file requested.
	 * @return The async request.
	 */
	FileRequestAsync* RequestFile(const std::string& folder_name, const std::string& file_name);

	/**
	 * Creates a request to a file.
	 * When the same file was already requested this will return an already
	 * existing request.
	 * The caller shouldn't choose different code path depending on new or
	 * existing requests.
	 *
	 * @param file_name Name of the requested file requested.
	 * @return The async request.
	 */
	FileRequestAsync* RequestFile(const std::string& file_name);

	/**
	 * Checks if any file with important-flag hasn't finished downloading yet.
	 *
	 * @return If any file with important-flag is pending.
	 */
	bool IsImportantFilePending();
}

using FileRequestPending = std::shared_ptr<int>;
using FileRequestPendingWeak = std::weak_ptr<int>;

/**
 * Handles a single asynchronous file request.
 */
class FileRequestAsync {
public:
	enum AsyncState {
		State_WaitForStart,
		State_DoneSuccess,
		State_DoneFailure,
		State_Pending
	};

	/**
	 * Don't use this API directly. Use AsyncHandler::RequestFile.
	 */
	FileRequestAsync();

	/**
	 * Don't use this API directly. Use AsyncHandler::RequestFile.
	 *
	 * @param folder_name folder where the file is stored.
	 * @param file_name Name of the requested file requested.
	 * @return The async request.
	 */
	FileRequestAsync(const std::string& folder_name, const std::string& file_name);

	/**
	 * Checks if a request finished.
	 *
	 * @return True when request suceeded or failed, false otherwise.
	 */
	bool IsReady() const;

	/**
	 * @return If while has important-flag set.
	 */
	bool IsImportantFile() const;

	/**
	 * Sets the important flag.
	 * This flag must be set before Start() is invoked.
	 * When the important flag is set the Player update loop will block until
	 * the request is finished.
	 *
	 * @param important value of important flag.
	 * @return 
	 */
	void SetImportantFile(bool important);

	/**
	 * Starts the async requests.
	 * When the request was already started earlier and is pending this call
	 * does nothing. When the request is already all binded event handlers are
	 * called immediately.
	 */
	void Start();

	/**
	 * @return Path to the requested file.
	 */
	const std::string& GetPath() const;

	/**
	 * Binds a member function as request-finished event handler.
	 * The event handler is only invoked once (one-shot).
	 *
	 * @param func member function.
	 * @param that instance of object.
	 * @return Event ID. 
	 */
	template<typename T> FileRequestPending Bind(void (T::*func)(FileRequestResult*), T* that);

	/**
	 * Binds a function as request-finished event handler.
	 * The event handler is only invoked once (one-shot).
	 *
	 * @param func function.
	 * @return Event ID.
	 */
	FileRequestPending Bind(boost::function<void(FileRequestResult*)> func);

	/**
	 * Binds a function as request-finished event handler.
	 * The event handler is only invoked once (one-shot).
	 *
	 * @param func function.
	 * @return Event ID.
	 */
	FileRequestPending Bind(void(*func)(FileRequestResult*));

	// don't call these directly
	void DownloadDone(bool success);
	void UpdateProgress();
private:
	void CallListeners(bool success);

	std::vector<std::pair<FileRequestPendingWeak, boost::function<void(FileRequestResult*)> > > listeners;
	std::string directory;
	std::string file;
	std::string path;
	int state;
	bool important;
};

/**
 * Contains the result of an async request.
 * directory: Directory name
 * file: Name of requested file
 * success: true if requested was successful, otherwise false.
 */
struct FileRequestResult {
	std::string directory;
	std::string file;
	bool success;
};

template<typename T>
FileRequestPending FileRequestAsync::Bind(void (T::*func)(FileRequestResult*), T* that) {
	boost::function1<void, FileRequestResult*> f;
	f = std::bind1st(std::mem_fun(func), that);
	return Bind(f);
}

#endif
