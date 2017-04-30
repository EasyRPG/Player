package org.easyrpg.player.button_mapping;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.view.MotionEvent;

import org.easyrpg.player.Helper;
import org.easyrpg.player.settings.SettingsManager;
import org.libsdl.app.SDLActivity;

public class FastForwardingButton extends VirtualButton {
    boolean alreadyActivated;

    protected FastForwardingButton(Context context, int keyCode, double posX, double posY, int size) {
        super(context, keyCode, posX, posY, size);
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

                if (!isPressAndReleaseBehaviorEnabled() || (isPressAndReleaseBehaviorEnabled() && alreadyActivated)) {
                    SDLActivity.onNativeKeyUp(this.keyCode);
                    alreadyActivated = false;
                } else {
                    alreadyActivated = true;
                }
            }
        }

    }

    private boolean isPressAndReleaseBehaviorEnabled(){
        return SettingsManager.isPressAndReleaseForFastForwardingEnabled();
    }
}
