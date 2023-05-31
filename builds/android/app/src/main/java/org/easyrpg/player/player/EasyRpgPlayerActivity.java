/*
 * This file is part of EasyRPG Player
 *
 * Copyright (c) 2017 EasyRPG Project. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

package org.easyrpg.player.player;

import android.app.AlertDialog;
import android.content.ClipDescription;
import android.content.Intent;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.StrictMode;
import android.text.SpannableString;
import android.text.method.LinkMovementMethod;
import android.text.util.Linkify;
import android.util.Log;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.RelativeLayout;
import android.widget.RelativeLayout.LayoutParams;
import android.widget.TextView;

import androidx.core.view.GravityCompat;
import androidx.drawerlayout.widget.DrawerLayout;

import com.google.android.material.navigation.NavigationView;

import org.easyrpg.player.Helper;
import org.easyrpg.player.R;
import org.easyrpg.player.button_mapping.ButtonMappingActivity;
import org.easyrpg.player.button_mapping.InputLayout;
import org.easyrpg.player.button_mapping.VirtualButton;
import org.easyrpg.player.game_browser.Game;
import org.easyrpg.player.game_browser.GameBrowserActivity;
import org.easyrpg.player.settings.SettingsManager;
import org.libsdl.app.SDLActivity;

import java.io.File;
import java.lang.reflect.Method;
import java.util.ArrayList;

/**
 * EasyRPG Player for Android (inheriting from SDLActivity)
 */
public class EasyRpgPlayerActivity extends SDLActivity implements NavigationView.OnNavigationItemSelectedListener {
    public static final String TAG_PROJECT_PATH = "project_path";
    public static final String TAG_SAVE_PATH = "save_path";
    public static final String TAG_COMMAND_LINE = "command_line";
    public static final String TAG_STANDALONE = "standalone_mode";
    public static final int LAYOUT_EDIT = 12345;

    public static boolean samsungMultitouchWorkaround = false;
    public static int pointerCount = 0;

    DrawerLayout drawer;
    InputLayout inputLayout;
    private boolean uiVisible = true;
    SurfaceView surface;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (mBrokenLibraries) {
            return;
        }

        SettingsManager.init(getApplicationContext());

        // Menu configuration
        this.drawer = findViewById(R.id.drawer_layout);
        drawer.setDrawerLockMode(DrawerLayout.LOCK_MODE_LOCKED_CLOSED);
        drawer.addDrawerListener(new DrawerLayout.DrawerListener() {
            @Override
            public void onDrawerSlide(View drawerView, float slideOffset) {
                // Prevent drawer from rendering behind the SDL Surface
                // via https://stackoverflow.com/q/20976149/
                drawer.bringChildToFront(drawerView);
                drawer.requestLayout();
            }

            @Override
            public void onDrawerOpened(View drawerView) {

            }

            @Override
            public void onDrawerClosed(View drawerView) {

            }

            @Override
            public void onDrawerStateChanged(int newState) {

            }
        });

        NavigationView navigationView = findViewById(R.id.nav_view);
        navigationView.setNavigationItemSelectedListener(this);
        hideStatusBar();

        // Screen orientation
        if (SettingsManager.isForcedLandscape()) {
            this.setOrientationBis(0, 0, false, "LandscapeRight|LandscapeLeft");
        }

        // Hardware acceleration
        try {
            // Api > 11: FLAG_HARDWARE_ACCELERATED
            // TODO : Is it still useful?
            getWindow().setFlags(0x01000000, 0x01000000);
        } catch (Exception e) {
            e.printStackTrace();
        }

        // Put the gameScreen
        surface = mSurface;
        mLayout = findViewById(R.id.main_layout);
        mLayout.addView(surface);
        updateScreenPosition();

        // Set speed multiplier
        setFastForwardMultiplier(SettingsManager.getFastForwardMultiplier());

        showInputLayout();
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        pointerCount = ev.getPointerCount();
        return super.dispatchTouchEvent(ev);
    }

    /**
     * Called after a screen orientation change
     */
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        updateScreenPosition();
        showInputLayout();
    }

    @Override
    public boolean onNavigationItemSelected(MenuItem item) {
        // Handle item selection
        if (item.getItemId() == R.id.open_settings) {
            openSettings();
        } else if (item.getItemId() == R.id.toggle_ui) {
            uiVisible = !uiVisible;
            showInputLayout();
        } else if (item.getItemId() == R.id.edit_layout) {
            editLayout();
        } else if (item.getItemId() == R.id.report_bug) {
            reportBug();
        } else if (item.getItemId() == R.id.end_game) {
            showEndGameDialog();
        }
        openOrCloseMenu();
        return false;
    }

    public void hideStatusBar() {
        // Hide the status bar
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == LAYOUT_EDIT) {
            showInputLayout();
        }
    }

    private void editLayout() {
        Intent intent = new Intent(this, org.easyrpg.player.button_mapping.ButtonMappingActivity.class);

        // Choose the proper InputLayout
        int display_mode = getResources().getConfiguration().orientation;
        if (display_mode == Configuration.ORIENTATION_LANDSCAPE) {
            intent.putExtra(ButtonMappingActivity.TAG_ORIENTATION, ButtonMappingActivity.TAG_ORIENTATION_VALUE_HORIZONTAL);
        } else {
            intent.putExtra(ButtonMappingActivity.TAG_ORIENTATION, ButtonMappingActivity.TAG_ORIENTATION_VALUE_VERTICAL);
        }

        for (VirtualButton v : inputLayout.getButtonList()) {
            mLayout.removeView(v);
        }

        startActivityForResult(intent, LAYOUT_EDIT);
    }

    private void reportBug() {
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
        alertDialogBuilder.setTitle(R.string.app_name);

        final SpannableString bug_msg = new SpannableString(getApplicationContext().getString(R.string.report_bug_msg));
        Linkify.addLinks(bug_msg, Linkify.ALL);

        // set dialog message
        alertDialogBuilder.setMessage(bug_msg).setCancelable(false)
                .setPositiveButton(R.string.ok, (dialog, id) -> {
                    // Attach to the email : the easyrpg log file and savefiles
                    ArrayList<Uri> files = new ArrayList<>();
                    // The easyrpg_log.txt
                    String savepath = getIntent().getStringExtra(TAG_SAVE_PATH);

                    if (getIntent().getBooleanExtra(TAG_STANDALONE, false)) {
                        // FIXME: Attaching files does not work because the files are in /data and
                        // other apps have no permission
                    } else {
                        Uri saveFolder = Uri.parse(savepath);
                        Uri log = Helper.findFileUri(getContext(), saveFolder, "easyrpg_log.txt");
                        if (log != null) {
                            files.add(log);
                        }
                        // The save files
                        files.addAll(Helper.findFileUriWithRegex(getContext(), saveFolder, ".*lsd"));
                    }

                    if (Build.VERSION.SDK_INT >= 24) {
                        // Lazy workaround as suggested on https://stackoverflow.com/q/38200282
                        try {
                            Method m = StrictMode.class.getMethod("disableDeathOnFileUriExposure");
                            m.invoke(null);
                        } catch (Exception e) {
                            Log.i("EasyRPG", "Bug report: Calling disableDeathOnFileUriExposure failed");
                        }
                    }

                    Intent intent = new Intent(Intent.ACTION_SEND_MULTIPLE);
                    // intent.setData(Uri.parse("mailto:"));
                    intent.setType(ClipDescription.MIMETYPE_TEXT_PLAIN);
                    intent.putExtra(Intent.EXTRA_EMAIL, new String[]{"easyrpg@easyrpg.org"});
                    intent.putExtra(Intent.EXTRA_SUBJECT, "Bug report");
                    intent.putExtra(Intent.EXTRA_TEXT, getApplicationContext().getString(R.string.report_bug_mail));
                    intent.putExtra(Intent.EXTRA_STREAM, files);
                    if (intent.resolveActivity(getPackageManager()) != null) {
                        startActivity(intent);
                    }
                }).setNegativeButton(R.string.cancel, (dialog, id) -> dialog.cancel());

        AlertDialog alertDialog = alertDialogBuilder.create();

        alertDialog.show();

        ((TextView) alertDialog.findViewById(android.R.id.message)).setMovementMethod(LinkMovementMethod.getInstance());
    }

    @Override
    public void onBackPressed() {
        openOrCloseMenu();
    }

    @Override
    public void setOrientationBis(int w, int h, boolean resizable, String hint) {
        // Filter orientation events generated by SDL (hint is empty)
        if (hint.isEmpty()) {
            return;
        }

        super.setOrientationBis(w, h, resizable, hint);
    }

    /**
     * This function permit to open the menu, in a static way
     */
    public static void staticOpenOrCloseMenu(EasyRpgPlayerActivity activity) {
        if (activity != null) {
            activity.openOrCloseMenu();
        }
    }

    public void openOrCloseMenu() {
        if (drawer.isDrawerOpen(GravityCompat.START)) {
            drawer.closeDrawer(GravityCompat.START);
        } else {
            drawer.openDrawer(GravityCompat.START);
        }
    }

    private void showEndGameDialog() {
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
        alertDialogBuilder.setTitle(R.string.app_name);

        // set dialog message
        alertDialogBuilder.setMessage(R.string.do_want_quit).setCancelable(false)
                .setPositiveButton(R.string.yes, (dialog, id) -> endGame()).setNegativeButton(R.string.no, (dialog, id) -> dialog.cancel());

        // create alert dialog
        AlertDialog alertDialog = alertDialogBuilder.create();

        alertDialog.show();
    }

    public static native void openSettings();

    public static native void endGame();

    public static native void setFastForwardMultiplier(int m);

    protected String[] getArguments() {
        return getIntent().getStringArrayExtra(TAG_COMMAND_LINE);
    }

    /**
     * Used to retrieve the selected game in the browser.
     *
     * @return Full path to game
     */
    public Game getProjectPath() {
        return GameBrowserActivity.getSelectedGame();
    }

    /**
     * Used by the native code to retrieve the RTP directory. Invoked via JNI.
     * DO NOT DELETE, IT IS USED BY THE NATIVE CODE (the warning is a lie)
     *
     * @return Full path to the RTP
     */
    public String getRtpPath() {
        if (SettingsManager.isRTPScanningEnabled()) {
            Uri rtpFolderURI = SettingsManager.getRTPFolderURI(this);
            if (rtpFolderURI != null) {
                return rtpFolderURI.toString();
            }
        }
        return "";
    }

    public SafFile getHandleForPath(String path) {
        return SafFile.fromPath(getContext(), path);
    }

    public void showInputLayout() {
        // Remove all buttons from the UI
        if (inputLayout != null) {
            for (VirtualButton b : inputLayout.getButtonList()) {
                mLayout.removeView(b);
            }
        }

        if (uiVisible) {
            // Retrieve the proper layout depending on the screen orientation
            int display_mode = getResources().getConfiguration().orientation;
            if (display_mode == Configuration.ORIENTATION_LANDSCAPE) {
                this.inputLayout = SettingsManager.getInputLayoutHorizontal(this);
            } else {
                this.inputLayout = SettingsManager.getInputLayoutVertical(this);
            }

            for (VirtualButton b : inputLayout.getButtonList()) {
                // We add it, if it's not the case already
                if (b.getParent() != mLayout) {
                    if (b.getParent() != null) {
                        ((ViewGroup) b.getParent()).removeAllViews();
                    }
                    mLayout.addView(b);
                }
            }

            for (VirtualButton b : this.inputLayout.getButtonList()) {
                Helper.setLayoutPosition(this, b, b.getPosX(), b.getPosY());
            }
        }
    }

    public void updateScreenPosition() {
        RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(LayoutParams.WRAP_CONTENT,
                LayoutParams.WRAP_CONTENT);
        int topMargin, leftMargin;

        // Determine the multiplier
        int screenWidth = getWindowManager().getDefaultDisplay().getWidth();
        params.topMargin = 0;
        params.leftMargin = 0;
        params.width = screenWidth;
        params.height = (int)(screenWidth * 0.75);
        surface.setLayoutParams(params);
    }

    /**
     * Used by the native APK-FS code to retrieve a handle to the asset manager.
     *
     * @return asset manager
     */
    public AssetManager getAssetManager() {
        return getAssets();
    }

    /**
     * Called after the activity is being re-displayed
     */
    @Override
    public void onRestart() {
        super.onRestart();
        updateScreenPosition();
        showInputLayout();
    }
}
