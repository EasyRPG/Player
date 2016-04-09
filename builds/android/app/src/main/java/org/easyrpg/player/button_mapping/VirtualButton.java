package org.easyrpg.player.button_mapping;

import org.easyrpg.player.Helper;
import org.easyrpg.player.SettingsActivity;
import org.libsdl.app.SDLActivity;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Vibrator;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;

public class VirtualButton extends View {
	private int keyCode;
	protected double posX, posY; // Relative position on the screen
	protected int originalSize, originalLetterSize, resizeFactor, realSize;
	private char charButton; // The char displayed on the button
	protected Paint painter;
	private Rect bound, letterBound = new Rect();
	protected boolean isPressed; // To know when the touch go out the button
	protected boolean debug_mode;
	Context context;
	Vibrator vibrator;

	public static final int DPAD = -1, ENTER = KeyEvent.KEYCODE_SPACE, CANCEL = KeyEvent.KEYCODE_B,
			SHIFT = KeyEvent.KEYCODE_SHIFT_LEFT, KEY_0 = KeyEvent.KEYCODE_0, KEY_1 = KeyEvent.KEYCODE_1,
			KEY_2 = KeyEvent.KEYCODE_2, KEY_3 = KeyEvent.KEYCODE_3, KEY_4 = KeyEvent.KEYCODE_4,
			KEY_5 = KeyEvent.KEYCODE_5, KEY_6 = KeyEvent.KEYCODE_6, KEY_7 = KeyEvent.KEYCODE_7,
			KEY_8 = KeyEvent.KEYCODE_8, KEY_9 = KeyEvent.KEYCODE_9, KEY_PLUS = 157, KEY_MINUS = 156, KEY_MULTIPLY = 155,
			KEY_DIVIDE = 154;

	public VirtualButton(Context context, int keyCode, double posX, double posY, int size) {
		super(context);
		this.context = context;
		vibrator = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);

		this.keyCode = keyCode;
		this.posX = posX;
		this.posY = posY;

		this.charButton = getAppropriateChar(keyCode);

		// Set UI properties
		painter = Helper.getUIPainter();

		// Base size: ~1 cm
		originalSize = Helper.getPixels(this, 60);
		originalLetterSize = Helper.getPixels(this, 25);

		// Retrieve the size factor
		if (SettingsActivity.IGNORE_LAYOUT_SIZE_SETTINGS) {
			this.resizeFactor = SettingsActivity.LAYOUT_SIZE;
		} else {
			this.resizeFactor = size;
		}
	}

	@Override
	protected void onDraw(Canvas canvas) {
		if (!debug_mode) {
			painter.setAlpha(255 - SettingsActivity.LAYOUT_TRANSPARENCY);
		}

		// Draw
		// The circle
		canvas.drawCircle(realSize / 2, realSize / 2, realSize / 2 - 5, painter);

		// The letter
		// Anticipate the size of the letter
		painter.setTextSize(Helper.getPixels(this, (int) (originalLetterSize * ((float) resizeFactor / 100))));
		painter.getTextBounds("" + charButton, 0, 1, letterBound);

		// Draw the letter, centered in the circle
		canvas.drawText("" + charButton, (realSize - letterBound.width()) / 2,
				letterBound.height() + (realSize - letterBound.height()) / 2, painter);
	}

	public int getFuturSize() {
		// Resize
		realSize = (int) ((float) originalSize * resizeFactor / 100); 

		return realSize;
	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		int s = getFuturSize();

		setMeasuredDimension(s, s);
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		if (debug_mode) {
			ButtonMappingActivity.dragVirtualButton(this, event);
		} else {
			bound = new Rect(this.getLeft(), this.getTop(), this.getRight(), this.getBottom());

			int action = event.getActionMasked();

			switch (action) {
			case (MotionEvent.ACTION_DOWN):
				onPressed();
				return true;
			case (MotionEvent.ACTION_UP):
				onReleased();
				return true;
			case (MotionEvent.ACTION_MOVE):
				if (!bound.contains(this.getLeft() + (int) event.getX(), this.getTop() + (int) event.getY())) {
					// User moved outside bounds
					onReleased();
				}
				return true;
			default:
				// return super.onTouchEvent(event);
				return true;
			}
		}

		return true;
	}

	public void onPressed() {
		if (!debug_mode) {
			if (!isPressed) {
				isPressed = true;

				SDLActivity.onNativeKeyDown(this.keyCode);
				// Vibration
				if (SettingsActivity.VIBRATION && vibrator != null) {
					vibrator.vibrate(SettingsActivity.VIBRATION_DURATION);
				}
			}
		}
	}

	public void onReleased() {
		if (!debug_mode) {
			// We only send a message to SDL Activity if the button is not
			// considered
			// released (in case the touch mouvement go out the button bounds)
			if (isPressed) {
				isPressed = false;
				SDLActivity.onNativeKeyUp(this.keyCode);
			}
		}
	}

	public char getAppropriateChar(int keyCode) {
		char charButton;

		if (keyCode == ENTER) {
			charButton = 'A';
		} else if (keyCode == CANCEL) {
			charButton = 'B';
		} else if (keyCode == SHIFT) {
			charButton = 'S';
		} else if (keyCode == KEY_0) {
			charButton = '0';
		} else if (keyCode == KEY_1) {
			charButton = '1';
		} else if (keyCode == KEY_2) {
			charButton = '2';
		} else if (keyCode == KEY_3) {
			charButton = '3';
		} else if (keyCode == KEY_4) {
			charButton = '4';
		} else if (keyCode == KEY_5) {
			charButton = '5';
		} else if (keyCode == KEY_6) {
			charButton = '6';
		} else if (keyCode == KEY_7) {
			charButton = '7';
		} else if (keyCode == KEY_8) {
			charButton = '8';
		} else if (keyCode == KEY_9) {
			charButton = '9';
		} else if (keyCode == KEY_MULTIPLY) {
			charButton = '*';
		} else if (keyCode == KEY_MINUS) {
			charButton = '-';
		} else if (keyCode == KEY_DIVIDE) {
			charButton = '/';
		} else if (keyCode == KEY_PLUS) {
			charButton = '+';
		} else {
			charButton = '?';
		}

		return charButton;
	}

	public double getPosX() {
		return posX;
	}

	public void setPosX(double posX) {
		this.posX = posX;
	}

	public double getPosY() {
		return posY;
	}

	public void setPosY(double posY) {
		this.posY = posY;
	}

	public int getKeyCode() {
		return keyCode;
	}

	public char getCharButton() {
		return charButton;
	}

	public int getSize() {
		return resizeFactor;
	}

	public void setSize(int size) {
		this.resizeFactor = size;
	}

	public void setDebug_mode(boolean debug_mode) {
		this.debug_mode = debug_mode;
	}

}
