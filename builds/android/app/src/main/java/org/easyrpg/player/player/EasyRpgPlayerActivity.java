/*
 * This file is part of EasyRPG Player
 *
 * Copyright (c) 2016 EasyRPG Project. All rights reserved.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 */

package org.easyrpg.player.player;

import java.io.File;
import java.util.ArrayList;

import org.easyrpg.player.Helper;
import org.easyrpg.player.R;
import org.easyrpg.player.SettingsActivity;
import org.easyrpg.player.button_mapping.ButtonMappingModel;
import org.easyrpg.player.button_mapping.ButtonMappingModel.InputLayout;
import org.easyrpg.player.button_mapping.VirtualButton;
import org.easyrpg.player.game_browser.GameBrowserHelper;
import org.easyrpg.player.game_browser.GameInformation;
import org.libsdl.app.SDLActivity;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.text.SpannableString;
import android.text.method.LinkMovementMethod;
import android.text.util.Linkify;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.SurfaceView;
import android.widget.RelativeLayout;
import android.widget.RelativeLayout.LayoutParams;
import android.widget.TextView;

/**
 * EasyRPG Player for Android (inheriting from SDLActivity)
 */

public class EasyRpgPlayerActivity extends SDLActivity {
	public static final String TAG_PROJECT_PATH = "project_path";
	public static final String TAG_SAVE_PATH = "save_path";
	public static final String TAG_COMMAND_LINE = "command_line";

	ButtonMappingModel bmm;
	InputLayout input_layout;
	private boolean uiVisible = true;
	SurfaceView surface;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		SettingsActivity.updateUserPreferences(getContext());

		// Hardware acceleration
		try {
			if (Build.VERSION.SDK_INT >= 11) {
				// Api 11: FLAG_HARDWARE_ACCELERATED
				getWindow().setFlags(0x01000000, 0x01000000);
			}
		} catch (Exception e) {
		}

		// Put the gamescreen
		surface = mSurface;
		mLayout = (RelativeLayout) findViewById(R.id.main_layout);
		mLayout.addView(surface);
		updateScreenPosition();

		// Open the proper input_layout
		bmm = ButtonMappingModel.getButtonMapping(this);

		// Project preferences
		GameInformation project = new GameInformation(getProjectPath());
		project.read_project_preferences_input_layout(bmm);

		// Choose the proper InputLayout
		input_layout = bmm.getLayoutById(this, project.getId_input_layout());

		// Add buttons
		addButtons();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.player, menu);
		Log.v("Player", "onCreateOption");
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle item selection
		switch (item.getItemId()) {
		case R.id.toggle_fps:
			toggleFps();
			return true;
		case R.id.toggle_ui:
			if (uiVisible) {
				for (VirtualButton v : input_layout.getButton_list()) {
					mLayout.removeView(v);
				}
				updateButtonsPosition();
			} else {
				addButtons();
			}
			uiVisible = !uiVisible;
			return true;
		case R.id.report_bug:
			reportBug();
			return true;
		case R.id.end_game:
			showEndGameDialog();
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
	}

	private void reportBug() {
		AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
		alertDialogBuilder.setTitle(R.string.app_name);
		
	    final SpannableString bug_msg = new SpannableString(getApplicationContext().getString(R.string.report_bug_msg));
	    Linkify.addLinks(bug_msg, Linkify.ALL);

		// set dialog message
		alertDialogBuilder.setMessage(bug_msg).setCancelable(false)
				.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int id) {						
						ArrayList<Uri> files = new ArrayList<Uri>();
						String savepath = getIntent().getStringExtra(TAG_SAVE_PATH);
						files.add(Uri.fromFile(new File(savepath + "/easyrpg_log.txt")));
						for (File f : GameBrowserHelper.getSavegames(new File(savepath))) {
							files.add(Uri.fromFile(f));
						}
						
					    Intent intent = new Intent(Intent.ACTION_SEND_MULTIPLE);
					    intent.setData(Uri.parse("mailto:"));
					    intent.setType("*/*");
					    intent.putExtra(Intent.EXTRA_EMAIL, new String[] {"easyrpg@easy-rpg.org"});
					    intent.putExtra(Intent.EXTRA_SUBJECT, "Bug report");
					    intent.putExtra(Intent.EXTRA_TEXT, getApplicationContext().getString(R.string.report_bug_mail));
					    intent.putExtra(Intent.EXTRA_STREAM, files);
					    if (intent.resolveActivity(getPackageManager()) != null) {
					        startActivity(intent);
					    }
					}
				}).setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int id) {
						dialog.cancel();
					}
				});

		AlertDialog alertDialog = alertDialogBuilder.create();

		alertDialog.show();
		
		((TextView)alertDialog.findViewById(android.R.id.message)).setMovementMethod(LinkMovementMethod.getInstance());
	}

	@Override
	public void onBackPressed() {
		openOptionsMenu();
	}

	/**
	 * This function prevents some Samsung's device to not show the option menu
	 */
	@Override
	public void openOptionsMenu() {

		Configuration config = getResources().getConfiguration();

		if ((config.screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK) > Configuration.SCREENLAYOUT_SIZE_LARGE) {

			int originalScreenLayout = config.screenLayout;
			config.screenLayout = Configuration.SCREENLAYOUT_SIZE_LARGE;
			super.openOptionsMenu();
			config.screenLayout = originalScreenLayout;

		} else {
			super.openOptionsMenu();
		}
	}

	private void showEndGameDialog() {
		AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
		alertDialogBuilder.setTitle(R.string.app_name);

		// set dialog message
		alertDialogBuilder.setMessage(R.string.do_want_quit).setCancelable(false)
				.setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int id) {
						endGame();
					}
				}).setNegativeButton(R.string.no, new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int id) {
						dialog.cancel();
					}
				});

		// create alert dialog
		AlertDialog alertDialog = alertDialogBuilder.create();

		alertDialog.show();
	}

	public static native void toggleFps();

	public static native void endGame();
	
    protected String[] getArguments() {
        return getIntent().getStringArrayExtra(TAG_COMMAND_LINE);
    }
    
	/**
	 * Used to retrieve the selected game in the browser.
	 * 
	 * @return Full path to game
	 */
	public String getProjectPath() {
		return getIntent().getStringExtra(TAG_PROJECT_PATH);
	}


	/**
	 * Used by timidity of SDL_mixer to find the timidity folder for the
	 * instruments. Invoked via JNI.
	 * 
	 * @return Full path to the timidity.cfg
	 */
	public String getTimidityPath() {
		// Log.v("SDL", "getTimidity " +
		// getApplication().getApplicationInfo().dataDir);
		String s = getApplication().getApplicationInfo().dataDir + "/timidity";
		if (new File(s).exists()) {
			return s;
		}

		return SettingsActivity.MAIN_DIRECTORY + "/timidity"; //Shouldn't be called anymore
	}

	/**
	 * Used by the native code to retrieve the RTP directory. Invoked via JNI.
	 * 
	 * @return Full path to the RTP
	 */
	public String getRtpPath() {
		String str = SettingsActivity.MAIN_DIRECTORY + "/rtp";
		// Log.v("SDL", "getRtpPath " + str);
		return str;
	}

	/**
	 * Gets the display height in pixel.
	 * 
	 * @return display height in pixel
	 */
	public int getScreenHeight() {
		DisplayMetrics displayMetrics = getResources().getDisplayMetrics();
		float screenWidthDp = displayMetrics.heightPixels;
		return (int) screenWidthDp;
	}

	/**
	 * Gets the display width in pixel.
	 * 
	 * @return display width in pixel
	 */
	public int getScreenWidth() {
		DisplayMetrics displayMetrics = getResources().getDisplayMetrics();
		float screenWidthDp = displayMetrics.widthPixels;
		return (int) screenWidthDp;
	}

	/**
	 * Draws all buttons.
	 */
	private void addButtons() {
		// Adding the buttons
		for (VirtualButton b : input_layout.getButton_list()) {
			// We add it, if it's not the case already
			if (b.getParent() != mLayout) {
				mLayout.addView(b);
			}
		}
		updateButtonsPosition();
	}

	public void updateButtonsPosition() {
		RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(LayoutParams.WRAP_CONTENT,
				LayoutParams.WRAP_CONTENT);

		int screenWidth = getWindowManager().getDefaultDisplay().getWidth();
		int screenHeight = getWindowManager().getDefaultDisplay().getHeight();

		for (VirtualButton b : input_layout.getButton_list()) {
			Helper.setLayoutPosition(this, b, b.getPosX(), b.getPosY());

			// We have to adjust the position in portrait configuration
			if (getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT) {
				params = (RelativeLayout.LayoutParams) b.getLayoutParams();
				// vertical : use approximatively the second part of the screen
				params.topMargin += (int) (screenHeight / 6);
				// horizontal : use a little gap to avoid button to be out of
				// the screen for button to the right
				if (b.getPosX() > 0.5) {
					params.leftMargin -= screenWidth / 8;
				}

				b.setLayoutParams(params);
			}
		}
	}

	public void updateScreenPosition() {
		RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(LayoutParams.WRAP_CONTENT,
				LayoutParams.WRAP_CONTENT);
		params.leftMargin = 0;

		if (getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT) {
			params.topMargin = -(getWindowManager().getDefaultDisplay().getHeight() / 2);
		} else {
			params.topMargin = 0;
		}
		surface.setLayoutParams(params);
	}

	/** Called after a screen orientation changement */
	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);

		updateScreenPosition();
		updateButtonsPosition();
	}
	
	/** Called after the activity is being re-displayed */
	@Override
	public void onRestart(){
		super.onRestart() ;
		updateScreenPosition();
		updateButtonsPosition();
	}	
	
}
