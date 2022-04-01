package org.easyrpg.player.button_mapping;

import android.app.Activity;
import android.graphics.Canvas;

import org.easyrpg.player.settings.SettingsManager;
import org.libsdl.app.SDLActivity;

public class FastForwardingButton extends VirtualButton {
    boolean alreadyActivated;

    protected FastForwardingButton(Activity activity, int keyCode, double posX, double posY, int size) {
        super(activity, keyCode, posX, posY, size);
    }

    @Override
    public void onPressed() {
        if (!debug_mode) {
            if (!isPressed) {
                isPressed = true;

                SDLActivity.onNativeKeyDown(this.keyCode);
                // Vibration
                if (SettingsManager.isVibrationEnabled() && vibrator != null) {
                    vibrator.vibrate(SettingsManager.getVibrationDuration());
                }
            }
        }
    }

    @Override
    public void onReleased() {
        if (!debug_mode) {
            if (isPressed) {
                isPressed = false;

                if (!isTapMode() || alreadyActivated) {
                    SDLActivity.onNativeKeyUp(this.keyCode);
                    alreadyActivated = false;
                } else {
                    alreadyActivated = true;
                }
            }
        }

    }

    @Override
    protected void onDraw(Canvas canvas) {
        setProperTransparency();

        // Draw the rectangle surrounding the button's letter
        int border = 5;
        canvas.drawRect(border, border, realSize - border, realSize - border, painter);

        // Draw the symbol, centered in the rectangle
        drawCenter(canvas, painter, String.valueOf(charButton));
    }

    private boolean isTapMode() {
        return (SettingsManager.getFastForwardMode() == SettingsManager.FAST_FORWARD_MODE_TAP);
    }
}
