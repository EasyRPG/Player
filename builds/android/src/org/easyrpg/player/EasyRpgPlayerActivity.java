package org.easyrpg.player;

import org.libsdl.app.SDLActivity;

import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.RelativeLayout.LayoutParams;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.TypedValue;
import android.content.res.Resources;
import android.graphics.*;
import android.graphics.Paint.Style;

/**
 * EasyRPG Player for Android (inheriting from SDLActivity)
 */

public class EasyRpgPlayerActivity extends SDLActivity {
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
	    mLayout = new RelativeLayout(this);
	    mLayout.addView(mSurface);
	    
	    drawButtons();
	    drawCross();
	
	    setContentView(mLayout);
	}
	
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
	 * Used by timidity of SDL_mixer to find the config file for the instruments.
	 * Invoked via JNI.
	 * 
	 * @return Full path to the timidity.cfg
	 */
	public String getTimidityConfigPath() {
		String str = Environment.getExternalStorageDirectory().getPath() + "/easyrpg/timidity/timidity.cfg";
		Log.v("SDL", "getTimidity " + str);
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
		ImageView aView = new ImageView(this);
		aView.setImageBitmap(abBmp);
		ImageView bView = new ImageView(this);
		bView.setImageBitmap(abBmp);
		setLayoutPosition(aView, 0.75, 0.8);
		setLayoutPosition(bView, 0.85, 0.7);
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
		ImageView cView = new ImageView(this);
		cView.setImageBitmap(cBmp);
		setLayoutPosition(cView, 0.03, 0.6);
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
    	params.leftMargin = getPixels(screenWidthDp * x); //Your X coordinate
    	params.topMargin = getPixels(screenHeightDp * y); //Your Y coordinate
    	view.setLayoutParams(params);        
	}
}
