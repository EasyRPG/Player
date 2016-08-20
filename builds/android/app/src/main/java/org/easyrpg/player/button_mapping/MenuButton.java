package org.easyrpg.player.button_mapping;

import android.content.Context;
import android.graphics.Canvas;

import org.easyrpg.player.player.EasyRpgPlayerActivity;
import org.easyrpg.player.settings.SettingsManager;

public class MenuButton extends VirtualButton {
    public static final int MENU_BUTTON_KEY = -2;


    public MenuButton(Context context, double posX, double posY, int size) {
        super(context, MENU_BUTTON_KEY, posX, posY, size);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (!debug_mode) {
            painter.setAlpha(255 - SettingsManager.getLayoutTransparency());
        }

        // The rectangle
        canvas.drawRect(0, 0, realSize, realSize, painter);
    }

    @Override
    public void onPressed() {
    }

    @Override
    public void onReleased() {
        if (!debug_mode) {
            ((EasyRpgPlayerActivity) context).openOptionsMenu();
        }
    }
}
