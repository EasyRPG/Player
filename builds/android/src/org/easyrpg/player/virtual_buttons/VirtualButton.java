package org.easyrpg.player.virtual_buttons;

import org.libsdl.app.SDLActivity;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Paint.Align;
import android.graphics.Rect;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

public class VirtualButton extends View {
	private int iconSize;
	private Paint painter;
	private Rect bound;
	private boolean isPressed; // To know when the touch go out the button
	private int keyCode;
	private char charButton; // The char displayed on the button

	public VirtualButton(Context context, int keyCode, char charButton) {
		super(context);

		this.keyCode = keyCode;
		// Set size
		iconSize = Utilitary.getPixels(this, 60); // ~1cm

		// Setup Painter and Button char
		painter = Utilitary.getUIPainter();
		this.charButton = charButton;
	}

	@Override
	protected void onDraw(Canvas canvas) {
		// Draw
		canvas.drawCircle(iconSize / 2, iconSize / 2, iconSize / 2 - 5, painter);
		painter.setTextSize(Utilitary.getPixels(this, 55));
		painter.setTextAlign(Align.CENTER);
		canvas.drawText("" + charButton, iconSize / 2, iconSize / 5 * 4,
				painter);
	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		setMeasuredDimension(iconSize, iconSize);
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		bound = new Rect(this.getLeft(), this.getTop(), this.getRight(),
				this.getBottom());

		int action = event.getActionMasked();

		switch (action) {
		case (MotionEvent.ACTION_DOWN):
			onPressed();
			return true;
		case (MotionEvent.ACTION_UP):
			onReleased();
			return true;
		case (MotionEvent.ACTION_MOVE):
			if (!bound.contains(this.getLeft() + (int) event.getX(),
					this.getTop() + (int) event.getY())) {
				// User moved outside bounds
				onReleased();
			}
			return true;
		default:
			// return super.onTouchEvent(event);
			return true;
		}
	}

	public void onPressed() {
		if (!isPressed) {
			isPressed = true;

			SDLActivity.onNativeKeyDown(this.keyCode);
		}
	}

	public void onReleased() {
		// We only send a message to SDL Activity if the button is not
		// considered
		// released (in case the touch mouvement go out the button bounds)
		if (isPressed) {
			isPressed = false;
			SDLActivity.onNativeKeyUp(this.keyCode);
		}
	}
}
