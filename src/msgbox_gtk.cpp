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

#ifndef GTK
    #error This build doesn't support GTK
#endif

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include "msgbox.h"

////////////////////////////////////////////////////////////
/// Default Message Box with OK button
////////////////////////////////////////////////////////////
void MsgBox::OK(std::string msg, std::string title) {
    GtkWidget *werror;

    gtk_init(NULL, NULL);
    werror = gtk_message_dialog_new(NULL,
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_CLOSE,
        "%s", msg.c_str());
    gtk_window_set_title(GTK_WINDOW(werror), title.c_str());
    gtk_dialog_run(GTK_DIALOG(werror));
    gtk_widget_destroy(werror);
}

////////////////////////////////////////////////////////////
/// Error Message Box
////////////////////////////////////////////////////////////
void MsgBox::Error(std::string msg, std::string title) {
    GtkWidget *werror;

    gtk_init(NULL, NULL);
    werror = gtk_message_dialog_new(NULL,
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_CLOSE,
        "%s", msg.c_str());
    gtk_window_set_title(GTK_WINDOW(werror), title.c_str());
    gtk_dialog_run(GTK_DIALOG(werror));
    gtk_widget_destroy(werror);
}

////////////////////////////////////////////////////////////
/// Warning Message Box
////////////////////////////////////////////////////////////
void MsgBox::Warning(std::string msg, std::string title) {
    GtkWidget *werror;

    gtk_init(NULL, NULL);
    werror = gtk_message_dialog_new(NULL,
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_CLOSE,
        "%s", msg.c_str());
    gtk_window_set_title(GTK_WINDOW(werror), title.c_str());
    gtk_dialog_run(GTK_DIALOG(werror));
    gtk_widget_destroy(werror);
}
