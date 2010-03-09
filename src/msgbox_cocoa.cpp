//////////////////////////////////////////////////////////////////////////////////
/// This file is part of EasyRPG Player.
/// 
/// EasyRPG Player is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// 
/// EasyRPG Player is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// You should have received a copy of the GNU General Public License
/// along with EasyRPG Player.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

#ifndef COCOA
    #error This build doesn't support cocoa
#endif

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include "msgbox.h"

////////////////////////////////////////////////////////////
/// Default Message Box with OK button
////////////////////////////////////////////////////////////
void MsgBox::OK(std::string msg, std::string title) {
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
void MsgBox::Error(std::string msg, std::string title) {
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
void MsgBox::Warning(std::string msg, std::string title) {
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
