package org.easyrpg.player.virtual_buttons;

import org.libsdl.app.SDLActivity;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.Rect;
import android.graphics.Paint.Style;
import android.text.InputFilter.LengthFilter;
import android.util.Log;
import android.util.TypedValue;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;

public class VirtualCross extends View {
	private int		iconSize;
	private Paint	painter;
	private Rect	bound, boundLeft, boundRight, boundUp,boundDown;
	private boolean isPressed; //To know when the touch go out the button
	private int		key_pressed;
	private Path 	path; //For the drawing

	public VirtualCross(Context context) {
		super(context);
		// Set size
		iconSize = Utilitary.getPixels(this, 150); // ~1cm
	
		// Setup color
		painter = Utilitary.getUIPainter();
		path = new Path();
		setBounds();
	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		setMeasuredDimension(iconSize, iconSize);
	}
	
	@Override
	protected void onDraw(Canvas canvas) {
		int iconSize_33 = (int)(iconSize * 0.33);
		// Draw the cross
		path.reset();
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
		canvas.drawPath(path, painter);
		
		painter.setColor(Color.CYAN);
		canvas.drawRect(boundLeft, painter);
		painter.setColor(Color.argb(128, 255, 255, 255));
	}
	
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		this.setBounds();
		
		int action = event.getActionMasked();
		int keyCode = -1;
		
		if(boundLeft.contains(this.getLeft() + (int) event.getX(), this.getTop() + (int) event.getY())){
			keyCode = KeyEvent.KEYCODE_DPAD_LEFT; 
        }
		else if(boundRight.contains(this.getLeft() + (int) event.getX(), this.getTop() + (int) event.getY())){
			keyCode = KeyEvent.KEYCODE_DPAD_RIGHT;
		}
		else if(boundUp.contains(this.getLeft() + (int) event.getX(), this.getTop() + (int) event.getY())){
			keyCode = KeyEvent.KEYCODE_DPAD_UP;
		}
		else if(boundDown.contains(this.getLeft() + (int) event.getX(), this.getTop() + (int) event.getY())){
			keyCode = KeyEvent.KEYCODE_DPAD_DOWN;
		}
		
		if(keyCode != 1){ 
			switch (action) {
			case (MotionEvent.ACTION_DOWN):
				onPressed(keyCode);
				return true;
			case (MotionEvent.ACTION_UP):
				onReleased();
				return true;
			case (MotionEvent.ACTION_MOVE):
		        if(keyCode != key_pressed)
		        	onPressed(keyCode);
				return true;
			default:
				return true;
			}
		}
		return true;
	}
	
	public void onPressed(int keyCode){
		if(isPressed && key_pressed != keyCode){
			// If the DPad is already pressed but the direction
			// is different, we have to "up" the previous key
			//Log.d("Button", key_pressed + "released");
			if(key_pressed != -1)
				SDLActivity.onNativeKeyUp(key_pressed);
		}
		//Log.d("Button", keyCode + "Pressed");
		if(keyCode != 1)
			SDLActivity.onNativeKeyDown(keyCode);
		key_pressed = keyCode;
		isPressed = true;
	}
	
	public void onReleased(){
		// We only send a message to SDL Activity if the button is not considered 
		// released (in case the touch mouvement go out the button bounds)
		if(isPressed){
			isPressed = false;
			//Log.d("Button", key_pressed + "Released");
			if(key_pressed != -1)
				SDLActivity.onNativeKeyUp(key_pressed);
		}
		key_pressed = -1;
		isPressed = false;
	}
	
	/** Set the direction's hitbox position */
	public void setBounds(){
		int iconSize_33 = (int)(iconSize * 0.33);
		bound = new Rect(this.getLeft(), this.getTop(), this.getRight(), this.getBottom());
		boundLeft 	=new Rect(	this.getLeft(),
								this.getTop()	+ iconSize_33,
								this.getRight()	- 2*iconSize_33,
								this.getBottom()- iconSize_33);
		boundRight	=new Rect(	this.getLeft()	+ 2*iconSize_33,
								this.getTop()	+ iconSize_33,
								this.getRight(),
								this.getBottom()- iconSize_33);
		boundUp 	=new Rect(	this.getLeft()	+ iconSize_33,
								this.getTop(),
								this.getRight()	- iconSize_33,
								this.getBottom()- 2*iconSize_33);
		boundDown 	=new Rect(	this.getLeft()	+ iconSize_33,
								this.getTop()	+ 2*iconSize_33,
								this.getRight()	-iconSize_33,
								this.getBottom());
	}
	
	
}
