package org.easyrpg.player.player;

import android.content.Context;
import android.view.SurfaceHolder;

import org.libsdl.app.SDLSurface;

public class EasyRpgSurface extends SDLSurface {
    private EasyRpgPlayerActivity activity = null;

    public EasyRpgSurface(Context context, EasyRpgPlayerActivity activity) {
        super(context);

        this.activity = activity;
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder,
                               int format, int width, int height) {
        // configurationChanged is not always sent because of Android bugs in various versions
        // SDL uses the surfaceChanged event to detect rotations instead
        super.surfaceChanged(holder, format, width, height);

        activity.updateScreenPosition();
    }
}
