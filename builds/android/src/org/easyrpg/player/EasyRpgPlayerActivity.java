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

package org.easyrpg.player;

import org.libsdl.app.SDLActivity;

import android.os.Bundle;
import android.os.Environment;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.RelativeLayout.LayoutParams;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.TypedValue;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.graphics.*;
import android.graphics.Paint.Style;

/**
 * EasyRPG Player for Android (inheriting from SDLActivity)
 */

public class EasyRpgPlayerActivity extends SDLActivity {
	ImageView aView, bView, cView;
	boolean uiVisible = true;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
	    mLayout = (RelativeLayout)findViewById(R.id.main_layout);
	    mLayout.addView(mSurface);
	    
	    drawButtons();
	    drawCross();
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
	    if (keyCode == KeyEvent.KEYCODE_BACK ) {
	        showEndGameDialog();
	    }
	    return super.onKeyDown(keyCode, event);
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    MenuInflater inflater = getMenuInflater();
	    inflater.inflate(R.menu.player_menu, menu);
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
	        		mLayout.removeView(aView);
	        		mLayout.removeView(bView);
	        		mLayout.removeView(cView);
	        	} else {
	        		mLayout.addView(aView);
	        		mLayout.addView(bView);
	        		mLayout.addView(cView);
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

	private void showEndGameDialog() {
		AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
		alertDialogBuilder.setTitle("EasyRPG Player");
    
		// set dialog message
		alertDialogBuilder
			.setMessage("Do you really want to quit?")
			.setCancelable(false)
			.setPositiveButton("Yes",new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog,int id) {
					endGame();
				}
			  })
			.setNegativeButton("No",new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog,int id) {
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
	 * Used by timidity of SDL_mixer to find the timidity folder for the instruments.
	 * Invoked via JNI.
	 * 
	 * @return Full path to the timidity.cfg
	 */
	public String getTimidityPath() {
		String str = Environment.getExternalStorageDirectory().getPath() + "/easyrpg/timidity";
		//Log.v("SDL", "getTimidity " + str);
		return str;
	}
	
	/**
	 * Used by the native code to retrieve the RTP directory.
	 * Invoked via JNI.
	 * 
	 * @return Full path to the RTP
	 */
	public String getRtpPath() {
		String str = Environment.getExternalStorageDirectory().getPath() + "/easyrpg/rtp";
		//Log.v("SDL", "getRtpPath " + str);
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
        return (int)screenWidthDp;
	}
	
	/**
	 * Gets the display width in pixel.
	 * 
	 * @return display width in pixel
	 */
	public int getScreenWidth() {
		DisplayMetrics displayMetrics = getResources().getDisplayMetrics();
        float screenWidthDp = displayMetrics.widthPixels;
        return (int)screenWidthDp;
	}
	
	/**
	 * Gets Painter used for ui drawing.
	 * 
	 * @return painter
	 */
	private Paint getPainter() {
		Paint uiPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		uiPaint.setColor(Color.argb(64, 255, 255, 255));
		uiPaint.setStyle(Style.STROKE);
		uiPaint.setStrokeWidth((float)3.0);
		return uiPaint;
	}
	
	/**
	 * Draws A and B button.
	 */
	private void drawButtons() {
		// Setup color
		Paint circlePaint = getPainter();
		
		// Set size
		int iconSize = getPixels(60); // ~1cm

		// Draw
		Bitmap abBmp = Bitmap.createBitmap(iconSize + 10, iconSize + 10, Bitmap.Config.ARGB_8888);
		Canvas c = new Canvas(abBmp);
		c.drawCircle(iconSize / 2, iconSize / 2, iconSize / 2 - 5, circlePaint);
		
		// Add to screen layout
		aView = new ImageView(this);
		aView.setImageBitmap(abBmp);
		bView = new ImageView(this);
		bView.setImageBitmap(abBmp);
		setLayoutPositionRight(aView, 0.13, 0.7);
		setLayoutPositionRight(bView, 0.03, 0.6);
		mLayout.addView(aView);
		mLayout.addView(bView);
	}
	
	/**
	 * Draws the digital cross.
	 */
	private void drawCross() {
		// Setup color
		Paint crossPaint = getPainter();
		
		// Set size
		int iconSize = getPixels(150); // ~2.5cm
		int iconSize_33 = (int)(iconSize * 0.33);
		
		// Draw the cross
		Bitmap cBmp = Bitmap.createBitmap(iconSize + 10, iconSize + 10, Bitmap.Config.ARGB_8888);
		Canvas c = new Canvas(cBmp);
		Path path = new Path();
		path.moveTo(iconSize_33, 5);
		path.lineTo(iconSize_33*2, 5);
		path.lineTo(iconSize_33*2, iconSize_33);
		path.lineTo(iconSize - 5, iconSize_33);
		path.lineTo(iconSize - 5, iconSize_33*2);
		path.lineTo(iconSize_33*2, iconSize_33*2);
		path.lineTo(iconSize_33*2, iconSize-5);
		path.lineTo(iconSize_33, iconSize-5);
		path.lineTo(iconSize_33, iconSize_33*2);
		path.lineTo(5, iconSize_33*2);
		path.lineTo(5, iconSize_33);
		path.lineTo(iconSize_33, iconSize_33);
		path.close();
		path.offset(0, 0);
		c.drawPath(path, crossPaint);
		
		// Add to screen layout
		cView = new ImageView(this);
		cView.setImageBitmap(cBmp);
		setLayoutPosition(cView, 0.03, 0.5);
		mLayout.addView(cView);
	}
	
	/**
	 * Converts density independent pixel to real screen pixel.
	 * 160 dip = 1 inch ~ 2.5 cm
	 * 
	 * @param dipValue dip
	 * @return pixel
	 */
    public int getPixels(double dipValue) { 
    	int dValue = (int)dipValue;
        Resources r = getResources();
        int px = (int)TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, dValue, r.getDisplayMetrics());
        return px; 
   }
    
    /**
     * Moves a view to a screen position.
     * Position is from 0 to 1 and converted to screen pixel.
     * Alignment is top left.
     * 
     * @param view View to move
     * @param x X position from 0 to 1
     * @param y Y position from 0 to 1
     */
	private void setLayoutPosition(View view, double x, double y) {
        DisplayMetrics displayMetrics = getResources().getDisplayMetrics();
        float screenWidthDp = displayMetrics.widthPixels / displayMetrics.density;
        float screenHeightDp = displayMetrics.heightPixels / displayMetrics.density;
        
    	RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(LayoutParams.WRAP_CONTENT,LayoutParams.WRAP_CONTENT);
    	params.leftMargin = getPixels(screenWidthDp * x);
    	params.topMargin = getPixels(screenHeightDp * y);
    	view.setLayoutParams(params);        
	}
	
    /**
     * Moves a view to a screen position.
     * Position is from 0 to 1 and converted to screen pixel.
     * Alignment is top right.
     * 
     * @param view View to move
     * @param x X position from 0 to 1
     * @param y Y position from 0 to 1
     */
	private void setLayoutPositionRight(View view, double x, double y) {
        DisplayMetrics displayMetrics = getResources().getDisplayMetrics();
        float screenWidthDp = displayMetrics.widthPixels / displayMetrics.density;
        float screenHeightDp = displayMetrics.heightPixels / displayMetrics.density;
        
    	RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(LayoutParams.WRAP_CONTENT,LayoutParams.WRAP_CONTENT);
    	params.addRule(RelativeLayout.ALIGN_PARENT_RIGHT, 1);
    	params.rightMargin = getPixels(screenWidthDp * x);
    	params.topMargin = getPixels(screenHeightDp * y);
    	view.setLayoutParams(params);        
	}
}
