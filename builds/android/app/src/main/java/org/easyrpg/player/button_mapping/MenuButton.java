package org.easyrpg.player.button_mapping;

import android.app.Activity;
import android.graphics.Canvas;

import org.easyrpg.player.player.EasyRpgPlayerActivity;
import org.easyrpg.player.settings.SettingsManager;

public class MenuButton extends VirtualButton {
    public static final int MENU_BUTTON_KEY = -2;


    public MenuButton(Activity activity, double posX, double posY, int size) {
        super(activity, MENU_BUTTON_KEY, posX, posY, size);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (!debug_mode) {
            painter.setAlpha(255 - SettingsManager.getLayoutTransparency());
        }

        // The rectangle
        int height = realSize / 7;
        for (int i = 0; i < 7; i++) {
            if (i % 2 == 1) {
                canvas.drawRect(realSize / 6f, i * height, (realSize * 5) / 6f, (i + 1) * height, painter);
            }
        }
    }

    @Override
    public void onPressed() {
        // Vibration
        if (!debug_mode) {
            if (!isPressed) {
                if (SettingsManager.isVibrationEnabled() && vibrator != null) {
                    vibrator.vibrate(SettingsManager.getVibrationDuration());
                }
            }
        }
    }

    @Override
    public void onReleased() {
        // Open the menu
        if (!debug_mode) {
            EasyRpgPlayerActivity.staticOpenOrCloseMenu((EasyRpgPlayerActivity) activity);
        }
    }
}
