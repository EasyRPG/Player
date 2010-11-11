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

#ifdef GTK

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <gtk/gtk.h>
#include "msgbox.h"
#include "SDL_syswm.h"
#include "player.h"

////////////////////////////////////////////////////////////
/// Default Message Box with OK button
////////////////////////////////////////////////////////////
void MsgBox::OK(std::string msg, std::string title) {
	SDL_SysWMinfo sdl_info;
	Window x_window_id;
	GtkWidget* werror;
	GdkWindow* parent_window;

	//Must leave full screen before gtk dialog show
	if (Player::fullscreen) {
		Player::StartVideoModeChange();
		Player::ToggleFullscreen();
		Player::EndVideoModeChange();
	}
	gtk_init(NULL, NULL);
	werror = gtk_message_dialog_new(NULL,
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_CLOSE,
		"%s", msg.c_str());
	gtk_window_set_title(GTK_WINDOW(werror), title.c_str());
	//Make possible use sdl window as gdk window to use as parent for dialog
	SDL_VERSION(&sdl_info.version);
	SDL_GetWMInfo(&sdl_info);
	x_window_id = sdl_info.info.x11.wmwindow;
	gtk_widget_realize(GTK_WIDGET(werror));
	parent_window = gdk_window_foreign_new(x_window_id);
	gdk_window_set_transient_for(GTK_WIDGET(werror)->window, parent_window);
	gtk_window_set_position(GTK_WINDOW(werror), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_dialog_run(GTK_DIALOG(werror));
	gtk_widget_destroy(werror);
}

////////////////////////////////////////////////////////////
/// Error Message Box
////////////////////////////////////////////////////////////
void MsgBox::Error(std::string msg, std::string title) {
	SDL_SysWMinfo sdl_info;
	Window x_window_id;
	GtkWidget* werror;
	GdkWindow* parent_window;

	//Must leave full screen before gtk dialog show
	if (Player::fullscreen)	{
		Player::StartVideoModeChange();
		Player::ToggleFullscreen();
		Player::EndVideoModeChange();
	}
	gtk_init(NULL, NULL);
	werror = gtk_message_dialog_new(NULL,
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_CLOSE,
		"%s", msg.c_str());
	gtk_window_set_title(GTK_WINDOW(werror), title.c_str());
	//Make possible use sdl window as gdk window to use as parent for dialog
	SDL_VERSION(&sdl_info.version);
	SDL_GetWMInfo(&sdl_info);
	x_window_id = sdl_info.info.x11.wmwindow;
	gtk_widget_realize(GTK_WIDGET(werror));
	parent_window = gdk_window_foreign_new(x_window_id);
	gdk_window_set_transient_for(GTK_WIDGET(werror)->window, parent_window);
	gtk_window_set_position(GTK_WINDOW(werror), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_dialog_run(GTK_DIALOG(werror));
	gtk_widget_destroy(werror);
}

////////////////////////////////////////////////////////////
/// Warning Message Box
////////////////////////////////////////////////////////////
void MsgBox::Warning(std::string msg, std::string title) {
	SDL_SysWMinfo sdl_info;
	Window x_window_id;
	GtkWidget* werror;
	GdkWindow* parent_window;

	//Must leave full screen before gtk dialog show
	if (Player::fullscreen) {
		Player::StartVideoModeChange();
		Player::ToggleFullscreen();
		Player::EndVideoModeChange();
	}
	gtk_init(NULL, NULL);
	werror = gtk_message_dialog_new(NULL,
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_WARNING,
		GTK_BUTTONS_CLOSE,
		"%s", msg.c_str());
	gtk_window_set_title(GTK_WINDOW(werror), title.c_str());
	//Make possible use sdl window as gdk window to use as parent for dialog
	SDL_VERSION(&sdl_info.version);
	SDL_GetWMInfo(&sdl_info);
	x_window_id = sdl_info.info.x11.wmwindow;
	gtk_widget_realize(GTK_WIDGET(werror));
	parent_window = gdk_window_foreign_new(x_window_id);
	gdk_window_set_transient_for(GTK_WIDGET(werror)->window, parent_window);
	gtk_window_set_position(GTK_WINDOW(werror), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_dialog_run(GTK_DIALOG(werror));
	gtk_widget_destroy(werror);
}

#endif
