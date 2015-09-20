package org.easyrpg.player;

import android.app.Activity;
import android.content.Context;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.util.DisplayMetrics;
import android.util.TypedValue;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.RelativeLayout.LayoutParams;
import android.widget.Toast;

public class Helper {
	/**
	 * Converts density independent pixel to real screen pixel. 
	 * 160 dip = 1 inch ~ 2.5 cm
	 * @param dipValue
	 *            dip
	 * @return pixel
	 */
	public static int getPixels(Resources r, double dipValue) {
		int dValue = (int) dipValue;
		int px = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP,
				dValue, r.getDisplayMetrics());
		return px;
	}

	public static int getPixels(View v, double dipValue) {
		return getPixels(v.getResources(), dipValue);
	}

	public static int getPixels(Activity v, double dipValue) {
		return getPixels(v.getResources(), dipValue);
	}

	/**
	 * Moves a view to a screen position. Position is from 0 to 1 and converted
	 * to screen pixel. Alignment is top left.
	 * @param view
	 *            View to move
	 * @param x
	 *            X position from 0 to 1
	 * @param y
	 *            Y position from 0 to 1
	 */
	public static void setLayoutPosition(Activity a, View view, double x,
			double y) {
		DisplayMetrics displayMetrics = a.getResources().getDisplayMetrics();
		float screenWidthDp = displayMetrics.widthPixels
				/ displayMetrics.density;
		float screenHeightDp = displayMetrics.heightPixels
				/ displayMetrics.density;

		RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(
				LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
		
		if (a.getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT) {
			//TODO : Modify the margin to get a good orientation in PORTRAIT 
			// Idea -> Use the half bottom of the screen 
			params.leftMargin = Helper.getPixels(a, screenWidthDp * x);
			params.topMargin = Helper.getPixels(a, screenHeightDp * y);
		} else {
			params.leftMargin = Helper.getPixels(a, screenWidthDp * x);
			params.topMargin = Helper.getPixels(a, screenHeightDp * y);
		}
		view.setLayoutParams(params);
	}

	/**
	 * Moves a view to a screen position. Position is from 0 to 1 and converted
	 * to screen pixel. Alignment is top right.
	 * @param view
	 *            View to move
	 * @param x
	 *            X position from 0 to 1
	 * @param y
	 *            Y position from 0 to 1
	 */
	public static void setLayoutPositionRight(Activity a, View view, double x,
			double y) {
		DisplayMetrics displayMetrics = a.getResources().getDisplayMetrics();
		float screenWidthDp = displayMetrics.widthPixels
				/ displayMetrics.density;
		float screenHeightDp = displayMetrics.heightPixels
				/ displayMetrics.density;

		RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(
				LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
		params.addRule(RelativeLayout.ALIGN_PARENT_RIGHT, 1);
		params.rightMargin = Helper.getPixels(a, screenWidthDp * x);
		params.topMargin = Helper.getPixels(a, screenHeightDp * y);
		view.setLayoutParams(params);
	}

	public static Paint getUIPainter() {
		Paint uiPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		uiPaint.setColor(Color.argb(128, 255, 255, 255));
		uiPaint.setStyle(Style.STROKE);
		uiPaint.setStrokeWidth((float) 3.0);
		return uiPaint;
	}
	
	public static void showWrongAPIVersion(Context context){
		Toast.makeText(context, "Not avaible on this API", Toast.LENGTH_SHORT).show();
	}
}
