package org.easyrpg.player;

import org.libsdl.app.SDLActivity;

/**
 * EasyRPG Player for Android (inheriting from SDLActivity)
 */

public class EasyRpgPlayerActivity extends SDLActivity {
	/**
	 * Used by the native code to retrieve the selected game in the browser.
	 * Invoked via JNI.
	 * 
	 * @return Full path to game
	 */
	public String getProjectPath() {
		return getIntent().getStringExtra("project_path");
	}
}
