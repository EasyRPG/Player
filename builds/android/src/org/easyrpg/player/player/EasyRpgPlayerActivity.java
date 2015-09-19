/*
 * This file is part of EasyRPG Player
 *
 * Copyright (c) 2013 EasyRPG Project. All rights reserved.
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
import java.util.List;

import org.easyrpg.player.Helper;
import org.easyrpg.player.R;
import org.easyrpg.player.R.id;
import org.easyrpg.player.R.menu;
import org.easyrpg.player.R.string;
import org.easyrpg.player.button_mapping.ButtonMappingModel;
import org.easyrpg.player.button_mapping.VirtualButton;
import org.libsdl.app.SDLActivity;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.res.Configuration;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.RelativeLayout;

/**
 * EasyRPG Player for Android (inheriting from SDLActivity)
 */

public class EasyRpgPlayerActivity extends SDLActivity {
	private List<VirtualButton> buttonList;
	private boolean uiVisible = true;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		try {
			if (Build.VERSION.SDK_INT >= 11) {
				// Api 11: FLAG_HARDWARE_ACCELERATED
				getWindow().setFlags(0x01000000, 0x01000000);
			}
		} catch (Exception e) { }

		mLayout = (RelativeLayout) findViewById(R.id.main_layout);
		mLayout.addView(mSurface);

		//TODO : Read a file, extract the good preset
		drawButtons();
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
				for(VirtualButton v : buttonList){
					mLayout.removeView(v);
				}
			} else {
				drawButtons();
			}
			uiVisible = !uiVisible;
			return true;
		case R.id.end_game:
			showEndGameDialog();
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
	}
	
	@Override
	public void onBackPressed(){
		openOptionsMenu();
	}

	/** This function prevents some Samsung's device to not show the option menu
	 */
	@Override
	public void openOptionsMenu() {

	    Configuration config = getResources().getConfiguration();

	    if((config.screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK) 
	            > Configuration.SCREENLAYOUT_SIZE_LARGE) {

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
		alertDialogBuilder.setTitle("EasyRPG Player");

		// set dialog message
		alertDialogBuilder
				.setMessage(R.string.do_want_quit)
				.setCancelable(false)
				.setPositiveButton(R.string.yes,
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog, int id) {
								endGame();
							}
						})
				.setNegativeButton(R.string.no,
						new DialogInterface.OnClickListener() {
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

	/**
	 * Used by the native code to retrieve the selected game in the browser.
	 * Invoked via JNI.
	 * 
	 * @return Full path to game
	 */
	public String getProjectPath() {
		return getIntent().getStringExtra("project_path");
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

		return Environment.getExternalStorageDirectory().getPath()
				+ "/easyrpg/timidity";
	}

	/**
	 * Used by the native code to retrieve the RTP directory. Invoked via JNI.
	 * 
	 * @return Full path to the RTP
	 */
	public String getRtpPath() {
		String str = Environment.getExternalStorageDirectory().getPath()
				+ "/easyrpg/rtp";
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
	private void drawButtons() {
		for(VirtualButton b : buttonList){
			Helper.setLayoutPosition(this, b, b.getPosX(), b.getPosY());
			mLayout.addView(b);
		}
	}
}
