package org.easyrpg.player.virtual_buttons;

import org.libsdl.app.SDLActivity;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.graphics.Rect;
import android.util.Log;
import android.util.TypedValue;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;


public class GameButton extends View {
	private int		iconSize;
	private Paint	painter;
	private Rect	bound;
	private boolean isPressed; //To know when the touch go out the button
	private int		keyCode;

	public GameButton(Context context, int keyCode) {
		super(context);

		this.keyCode = keyCode;
		// Set size
		iconSize = getPixels(60); // ~1cm

		// Setup color
		painter = getPainter();
	}

	@Override
	protected void onDraw(Canvas canvas) {
		// Draw
		canvas.drawCircle(iconSize / 2, iconSize / 2, iconSize / 2 - 5, painter);
	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		setMeasuredDimension(iconSize, iconSize);
	}

	public int getPixels(double dipValue) {
		int dValue = (int) dipValue;
		Resources r = getResources();
		int px = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP,
				dValue, r.getDisplayMetrics());
		return px;
	}

	private Paint getPainter() {
		Paint uiPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		uiPaint.setColor(Color.argb(128, 255, 255, 255));
		uiPaint.setStyle(Style.STROKE);
		uiPaint.setStrokeWidth((float) 3.0);
		return uiPaint;
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		bound = new Rect(this.getLeft(), this.getTop(), this.getRight(), this.getBottom());
		
		//DEMANDER L'API 15 pour prendre en compte tous les type de motionEvent
		int action = event.getActionMasked();
		
		switch (action) {
		case (MotionEvent.ACTION_DOWN):
			onPressed();
			return true;
		case (MotionEvent.ACTION_UP):
			onReleased();
			return true;
		case (MotionEvent.ACTION_MOVE):
	        if(!bound.contains(this.getLeft() + (int) event.getX(), this.getTop() + (int) event.getY())){
	            // User moved outside bounds
	        	onReleased();
	        }
			return true;
		default:
			//return super.onTouchEvent(event);
			return true;
		}		
	}
	
	public void onPressed(){
		if(!isPressed){
			isPressed = true;
			Log.d("Button", "Pressed");
			
			SDLActivity.onNativeKeyDown(this.keyCode);
		}
	}
	
	public void onReleased(){
		// We only send a message to SDL Activity if the button is not considered 
		// released (in case the touch mouvement go out the button bounds)
		if(isPressed){
			isPressed = false;
			Log.d("Button", "Released");
			SDLActivity.onNativeKeyUp(this.keyCode);
		}	
	}
}
