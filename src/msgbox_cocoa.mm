/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifdef COCOA

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "msgbox.h"
#import <Cocoa/Cocoa.h>


// Note to OSX hackers:
// Cocoa guidelines says only caution icon should be used when the operation is risky (data loss)
// So please keep the same app dialog icon as is for warning and error.

////////////////////////////////////////////////////////////
/// Default Message Box with OK button
////////////////////////////////////////////////////////////
void MsgBox::OK(std::string const& msg, std::string const& title) {
	//Objective-Crap(TM) needed for Cocoa's NSAlert. Yeah, 64-bit OSX doesn't have Carbon anymore.
	//TODO: totally untested yet
	NSString *nsTitle = [NSString stringWithUTF8String:title.c_str()];
	NSString *nsMessage = [NSString stringWithUTF8String:msg.c_str()];
	NSAlert *alert = [NSAlert alertWithMessageText:nsTitle
		defaultButton:@"OK"
		alternateButton:nil
		otherButton:nil
		informativeTextWithFormat:nsMessage];
	[alert runModal];
}

////////////////////////////////////////////////////////////
/// Error Message Box
////////////////////////////////////////////////////////////
void MsgBox::Error(std::string const& msg, std::string const& title) {
	//Objective-Crap(TM) needed for Cocoa's NSAlert. Yeah, 64-bit OSX doesn't have Carbon anymore.
	//TODO: totally untested yet
	NSString *nsTitle = [NSString stringWithUTF8String:title.c_str()];
	NSString *nsMessage = [NSString stringWithUTF8String:msg.c_str()];
	NSAlert *alert = [NSAlert alertWithMessageText:nsTitle
		defaultButton:@"OK"
		alternateButton:nil
		otherButton:nil
		informativeTextWithFormat:nsMessage];
	[alert runModal];
}

////////////////////////////////////////////////////////////
/// Warning Message Box
////////////////////////////////////////////////////////////
void MsgBox::Warning(std::string const& msg, std::string const& title) {
	//Objective-Crap(TM) needed for Cocoa's NSAlert. Yeah, 64-bit OSX doesn't have Carbon anymore.
	//TODO: totally untested yet
	NSString *nsTitle = [NSString stringWithUTF8String:title.c_str()];
	NSString *nsMessage = [NSString stringWithUTF8String:msg.c_str()];
	NSAlert *alert = [NSAlert alertWithMessageText:nsTitle
		defaultButton:@"OK"
		alternateButton:nil
		otherButton:nil
		informativeTextWithFormat:nsMessage];
	[alert runModal];
}

#endif
