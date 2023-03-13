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

#ifndef EP_ASYNC_HANDLER_H
#define EP_ASYNC_HANDLER_H

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "string_view.h"

class FileRequestAsync;
struct FileRequestResult;

/**
 * AsyncHandler supports asynchronous file requests for platforms that don't
 * support synchronous IO (e.g. Emscripten).
 */
namespace AsyncHandler {
	/**
	 * Parses the specified JSON file. The file mapping read from this file
	 * will be used for further ajax requests.
	 */
	void CreateRequestMapping(const std::string& file);

	/**
	 * Clears all requests. They will hit the server again.
	 * Called after changing the language to ensure the assets are replaced.
	 */
	void ClearRequests();

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
	FileRequestAsync* RequestFile(StringView folder_name, StringView file_name);

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
	FileRequestAsync* RequestFile(StringView file_name);

	/**
	 * Checks if any file with important-flag hasn't finished downloading yet.
	 *
	 * @return If any file with important-flag is pending.
	 */
	bool IsImportantFilePending();

	/**
	 * Checks if any file with graphic-flag hasn't finished downloading yet.
	 *
	 * @return If any file with graphic-flag is pending.
	 */
	bool IsGraphicFilePending();

	/**
	 * Checks if any file is hasn't finished downloading yet.
	 *
	 * @param important check only important files
	 * @param graphic check only graphic files
	 *
	 * @return If any file with params is pending.
	 */
	bool IsFilePending(bool important, bool graphic);
}

using FileRequestBinding = std::shared_ptr<int>;
using FileRequestBindingWeak = std::weak_ptr<int>;

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
	 *
	 * @param path path to the file
	 * @return The async request.
	 */
	FileRequestAsync(std::string path, std::string directory, std::string file);

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
	 */
	void SetImportantFile(bool important);

	/**
	 * @return If while has graphic-flag set.
	 */
	bool IsGraphicFile() const;

	/**
	 * Sets the graphic flag.
	 * When the graphic flag is set, Scene_Map will block starting any
	 * user requested transitions until the request is finished.
	 *
	 * @param important value of important flag.
	 */
	void SetGraphicFile(bool graphic);

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
	 * The caller must hold a reference to the returned object, otherwise the
	 * handler is unbound.
	 *
	 * @param func member function.
	 * @param that instance of object.
	 * @param args arguments passed to func.
	 * @return request binding reference.
	 */
	template<typename T, typename... Args>
	FileRequestBinding Bind(void (T::*func)(FileRequestResult*, Args...), T* that, Args... args);

	/**
	 * Binds a function as request-finished event handler.
	 * The event handler is only invoked once (one-shot).
	 * The caller must hold a reference to the returned object, otherwise the
	 * handler is unbound.
	 *
	 * @param func function.
	 * @return request binding reference.
	 */
	FileRequestBinding Bind(std::function<void(FileRequestResult*)> func);

	/**
	 * Binds a function as request-finished event handler.
	 * The event handler is only invoked once (one-shot).
	 * The caller must hold a reference to the returned object, otherwise the
	 * handler is unbound.
	 *
	 * @param func function.
	 * @return request binding reference.
	 */
	FileRequestBinding Bind(void(*func)(FileRequestResult*));

	// don't call these directly
	void DownloadDone(bool success);
	void UpdateProgress();
private:
	void CallListeners(bool success);

	std::vector<std::pair<FileRequestBindingWeak, std::function<void(FileRequestResult*)> > > listeners;
	std::string directory;
	std::string file;
	std::string path;
	int state = State_DoneFailure;
	bool important = false;
	bool graphic = false;
};

/**
 * Contains the result of an async request.
 * directory: Directory name
 * file: Name of requested file
 * request_id assigned to the callback that fired
 * success: true if requested was successful, otherwise false.
 */
struct FileRequestResult {
	const std::string& directory;
	const std::string& file;
	int request_id;
	bool success;
};

template<typename T, typename... Args>
FileRequestBinding FileRequestAsync::Bind(void (T::*func)(FileRequestResult*, Args...), T* that, Args... args) {
	std::function<void(FileRequestResult*)> f = std::bind(std::mem_fn(func), that, std::placeholders::_1, args...);
	return Bind(f);
}

inline bool FileRequestAsync::IsReady() const {
	return state == State_DoneSuccess || state == State_DoneFailure;
}

inline bool FileRequestAsync::IsImportantFile() const {
	return important;
}

inline void FileRequestAsync::SetImportantFile(bool important) {
	this->important = important;
}

inline bool FileRequestAsync::IsGraphicFile() const {
	return graphic;
}

inline const std::string& FileRequestAsync::GetPath() const {
	return path;
}


#endif
