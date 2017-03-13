package org.easyrpg.player.button_mapping;

import android.content.Context;
import android.graphics.Canvas;

import org.easyrpg.player.Helper;
import org.easyrpg.player.settings.SettingsManager;

/**
 * Created by gabriel on 10.03.17.
 */

public class VirtualButtonRectangle extends VirtualButton {
    protected VirtualButtonRectangle(Context context, int keyCode, double posX, double posY, int size) {
        super(context, keyCode, posX, posY, size);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (!debug_mode) {
            painter.setAlpha(255 - SettingsManager.getLayoutTransparency());
        }

        // Draw
        // The reactangle
        canvas.drawRect(0, 0, realSize, realSize, painter);

        // The letter
        // Anticipate the size of the letter
        painter.setTextSize(Helper.getPixels(this, (int) (originalLetterSize * ((float) resizeFactor / 100))));
        painter.getTextBounds("" + charButton, 0, 1, letterBound);

        // Draw the letter, centered in the circle
        canvas.drawText("" + charButton, (realSize - letterBound.width()) / 2,
                letterBound.height() + (realSize - letterBound.height()) / 2, painter);
    }
}
