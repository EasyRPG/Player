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

// This file exists to shield the rest of the codebase from the craziness
// that occurs when you #import <Foundation/Foundation.h>...
#include "system.h"

#ifdef __APPLE__
#import <Foundation/Foundation.h>
#include <string>
#include "platform/macos/utils.h"

std::string MacOSUtils::GetBundleDir() {
    std::string path;
    @autoreleasepool {
        NSBundle *mainBundle = [NSBundle mainBundle];
        NSURL *bundleURL = [mainBundle bundleURL];
        if ([[bundleURL pathExtension] isEqualToString: @"app"]) {
            bundleURL = [bundleURL URLByDeletingLastPathComponent];
        }
        const char* fsPath = [bundleURL fileSystemRepresentation];
        path.assign(fsPath);
    }
    return path;
}

#endif
