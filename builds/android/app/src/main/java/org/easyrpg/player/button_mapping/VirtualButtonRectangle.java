package org.easyrpg.player.button_mapping;

import android.content.Context;
import android.graphics.Canvas;

import org.easyrpg.player.Helper;
import org.easyrpg.player.settings.SettingsManager;

public class VirtualButtonRectangle extends VirtualButton {
    protected VirtualButtonRectangle(Context context, int keyCode, double posX, double posY, int size) {
        super(context, keyCode, posX, posY, size);
    }
}
