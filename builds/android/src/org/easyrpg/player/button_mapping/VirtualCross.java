package org.easyrpg.player.button_mapping;

import org.easyrpg.player.Helper;
import org.easyrpg.player.UserSettingActivity;
import org.libsdl.app.SDLActivity;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Path;
import android.graphics.Rect;
import android.view.KeyEvent;
import android.view.MotionEvent;

public class VirtualCross extends VirtualButton {
	private Rect boundLeft, boundRight, boundUp, boundDown;
	private int key_pressed;
	private Path path = new Path(); // For the drawing

	public VirtualCross(Context context, double posX, double posY, int size) {
		super(context, VirtualButton.DPAD, posX, posY, size);

		// Set size
		realSize = Helper.getPixels(this, 150); // ~1cm
	}

	@Override
	protected void onDraw(Canvas canvas) {
		int iconSize_33 = (int) (realSize * 0.33);
		// Draw the cross
		path.reset();
		path.moveTo(iconSize_33, 5);
		path.lineTo(iconSize_33 * 2, 5);
		path.lineTo(iconSize_33 * 2, iconSize_33);
		path.lineTo(realSize - 5, iconSize_33);
		path.lineTo(realSize - 5, iconSize_33 * 2);
		path.lineTo(iconSize_33 * 2, iconSize_33 * 2);
		path.lineTo(iconSize_33 * 2, realSize - 5);
		path.lineTo(iconSize_33, realSize - 5);
		path.lineTo(iconSize_33, iconSize_33 * 2);
		path.lineTo(5, iconSize_33 * 2);
		path.lineTo(5, iconSize_33);
		path.lineTo(iconSize_33, iconSize_33);
		path.close();
		path.offset(0, 0);
		canvas.drawPath(path, painter);
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		if (debug_mode) {
			ButtonMappingActivity.dragVirtualButton(this, event);
		} else {
			setBounds();
			int action = event.getActionMasked();
			int keyCode = -1;

			if (boundLeft.contains(this.getLeft() + (int) event.getX(), this.getTop() + (int) event.getY())) {
				keyCode = KeyEvent.KEYCODE_DPAD_LEFT;
			} else if (boundRight.contains(this.getLeft() + (int) event.getX(), this.getTop() + (int) event.getY())) {
				keyCode = KeyEvent.KEYCODE_DPAD_RIGHT;
			} else if (boundUp.contains(this.getLeft() + (int) event.getX(), this.getTop() + (int) event.getY())) {
				keyCode = KeyEvent.KEYCODE_DPAD_UP;
			} else if (boundDown.contains(this.getLeft() + (int) event.getX(), this.getTop() + (int) event.getY())) {
				keyCode = KeyEvent.KEYCODE_DPAD_DOWN;
			}

			if (keyCode != 1) {
				switch (action) {
				case (MotionEvent.ACTION_DOWN):
					onPressed(keyCode);
					return true;
				case (MotionEvent.ACTION_UP):
					onReleased();
					return true;
				case (MotionEvent.ACTION_MOVE):
					if (keyCode != key_pressed)
						onPressed(keyCode);
					return true;
				default:
					return true;
				}
			}
		}
		return true;
	}

	public void onPressed(int keyCode) {
		if (!debug_mode) {
			if (isPressed && key_pressed != keyCode) {
				// If the DPad is already pressed but the direction
				// is different, we have to "up" the previous key
				if (key_pressed != -1) {
					sendSDLUpMessage(key_pressed);
				}
			}
			if (keyCode != -1) {
				sendSDLDownMessage(keyCode);
				
				if(UserSettingActivity.VIBRATION){
					vibrator.vibrate(UserSettingActivity.VIBRATION_DURATION);
				}
			}
			key_pressed = keyCode;
			isPressed = true;
		}
	}

	public void onReleased() {
		if (!debug_mode) {
			// We only send a message to SDL Activity if the button is not
			// considered
			// released (in case the touch mouvement go out the button bounds)
			if (isPressed) {
				isPressed = false;
				if (key_pressed != -1) {
					sendSDLUpMessage(key_pressed);
				}
			}
			key_pressed = -1;
			isPressed = false;
		}
	}

	public void sendSDLDownMessage(int keycode) {
		// Log.i("Cross", keycode + " pressed.");
		SDLActivity.onNativeKeyDown(keycode);
	}

	public void sendSDLUpMessage(int keycode) {
		// Log.i("Cross", keycode + " released.");
		SDLActivity.onNativeKeyUp(keycode);
	}

	/** Set the direction's hitbox position */
	public void setBounds() {
		int iconSize_33 = (int) (realSize * 0.33);
		int padding = (int) (realSize * 0.20); // We use it to slightly increase
												// hitboxs
		boundLeft = new Rect(this.getLeft() - padding, this.getTop() + iconSize_33, this.getRight() - 2 * iconSize_33,
				this.getBottom() - iconSize_33 + padding);
		boundRight = new Rect(this.getLeft() + 2 * iconSize_33, this.getTop() + iconSize_33, this.getRight() + padding,
				this.getBottom() - iconSize_33 + padding);
		boundUp = new Rect(this.getLeft() + iconSize_33, this.getTop() - padding, this.getRight() - iconSize_33,
				this.getBottom() - 2 * iconSize_33);
		boundDown = new Rect(this.getLeft() + iconSize_33, this.getTop() + 2 * iconSize_33,
				this.getRight() - iconSize_33, this.getBottom() + padding);
	}
}
