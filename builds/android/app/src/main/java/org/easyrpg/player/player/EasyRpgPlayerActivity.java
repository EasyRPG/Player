/*
 * This file is part of EasyRPG Player
 *
 * Copyright (c) 2016 EasyRPG Project. All rights reserved.
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
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.support.design.widget.NavigationView;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v4.widget.DrawerLayout.DrawerListener;
import android.text.SpannableString;
import android.text.method.LinkMovementMethod;
import android.text.util.Linkify;
import android.util.DisplayMetrics;
import android.view.MenuItem;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.RelativeLayout;
import android.widget.RelativeLayout.LayoutParams;
import android.widget.TextView;

import org.easyrpg.player.Helper;
import org.easyrpg.player.R;
import org.easyrpg.player.button_mapping.ButtonMappingActivity;
import org.easyrpg.player.button_mapping.ButtonMappingManager;
import org.easyrpg.player.button_mapping.ButtonMappingManager.InputLayout;
import org.easyrpg.player.button_mapping.VirtualButton;
import org.easyrpg.player.game_browser.GameBrowserHelper;
import org.easyrpg.player.game_browser.GameInformation;
import org.easyrpg.player.settings.SettingsManager;
import org.libsdl.app.SDLActivity;

import java.io.File;
import java.util.ArrayList;

/**
 * EasyRPG Player for Android (inheriting from SDLActivity)
 */

public class EasyRpgPlayerActivity extends SDLActivity implements NavigationView.OnNavigationItemSelectedListener {
    public static final String TAG_PROJECT_PATH = "project_path";
    public static final String TAG_SAVE_PATH = "save_path";
    public static final String TAG_COMMAND_LINE = "command_line";
    public static final int LAYOUT_EDIT = 12345;

    private static EasyRpgPlayerActivity instance;

    DrawerLayout drawer;
    ButtonMappingManager buttonMappingManager;
    InputLayout inputLayout;
    private boolean uiVisible = true;
    SurfaceView surface;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EasyRpgPlayerActivity.instance = this;

        SettingsManager.init(getApplicationContext());

        // Menu configuration
        this.drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
        drawer.setDrawerLockMode(DrawerLayout.LOCK_MODE_LOCKED_CLOSED);

        drawer.setDrawerListener(new DrawerListener() {
            @Override
            public void onDrawerSlide(View view, float arg1) {
                drawer.bringChildToFront(view);
                drawer.requestLayout();
            }

            @Override public void onDrawerStateChanged(int arg0) {}
            @Override public void onDrawerOpened(View arg0) {}
            @Override public void onDrawerClosed(View arg0) {}
        });

        NavigationView navigationView = (NavigationView) findViewById(R.id.nav_view);
        navigationView.setNavigationItemSelectedListener(this);
        hideStatusBar();

        // Screen orientation
        if (SettingsManager.isForcedLandscape()) {
            this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }

        // Hardware acceleration
        try {
            if (Build.VERSION.SDK_INT >= 11) {
                // Api 11: FLAG_HARDWARE_ACCELERATED
                getWindow().setFlags(0x01000000, 0x01000000);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        // Put the gamescreen
        surface = mSurface;
        mLayout = (RelativeLayout) findViewById(R.id.main_layout);
        mLayout.addView(surface);
        updateScreenPosition();

        // Project preferences
        buttonMappingManager = ButtonMappingManager.getInstance(this);
        GameInformation project = new GameInformation(getProjectPath());
        project.getProjectInputLayout(buttonMappingManager);

        // Choose the proper InputLayout
        inputLayout = buttonMappingManager.getLayoutById(project.getId_input_layout());

        // Add buttons
        addButtons();
    }

    @Override
    public boolean onNavigationItemSelected(MenuItem item) {
        // Handle item selection
        switch (item.getItemId()) {
            case R.id.toggle_fps:
                toggleFps();
                break;
            case R.id.toggle_ui:
                if (uiVisible) {
                    for (VirtualButton v : inputLayout.getButtonList()) {
                        mLayout.removeView(v);
                    }
                    updateButtonsPosition();
                } else {
                    addButtons();
                }
                uiVisible = !uiVisible;
                break;
            case R.id.edit_layout:
                editLayout();
                break;
            case R.id.report_bug:
                reportBug();
                break;
            case R.id.end_game:
                showEndGameDialog();
                break;
            default:
                return false;
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
            GameInformation project = new GameInformation(getProjectPath());
            project.getProjectInputLayout(buttonMappingManager);

            // Choose the proper InputLayout
            inputLayout = buttonMappingManager.getLayoutById(project.getId_input_layout());

            // Add buttons
            addButtons();
        }
    }
    
    private void editLayout() {
        Intent intent = new Intent(this, org.easyrpg.player.button_mapping.ButtonMappingActivity.class);
        GameInformation project = new GameInformation(getProjectPath());
        project.getProjectInputLayout(buttonMappingManager);

        // Choose the proper InputLayout
        intent.putExtra(ButtonMappingActivity.TAG_ID, project.getId_input_layout());
        
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
                .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        ArrayList<Uri> files = new ArrayList<Uri>();
                        String savepath = getIntent().getStringExtra(TAG_SAVE_PATH);
                        files.add(Uri.fromFile(new File(savepath + "/easyrpg_log.txt")));
                        for (File f : GameBrowserHelper.getSavegames(new File(savepath))) {
                            files.add(Uri.fromFile(f));
                        }

                        Intent intent = new Intent(Intent.ACTION_SEND_MULTIPLE);
                        intent.setData(Uri.parse("mailto:"));
                        intent.setType("*/*");
                        intent.putExtra(Intent.EXTRA_EMAIL, new String[]{"easyrpg@easyrpg.org"});
                        intent.putExtra(Intent.EXTRA_SUBJECT, "Bug report");
                        intent.putExtra(Intent.EXTRA_TEXT, getApplicationContext().getString(R.string.report_bug_mail));
                        intent.putExtra(Intent.EXTRA_STREAM, files);
                        if (intent.resolveActivity(getPackageManager()) != null) {
                            startActivity(intent);
                        }
                    }
                }).setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                dialog.cancel();
            }
        });

        AlertDialog alertDialog = alertDialogBuilder.create();

        alertDialog.show();

        ((TextView) alertDialog.findViewById(android.R.id.message)).setMovementMethod(LinkMovementMethod.getInstance());
    }

    @Override
    public void onBackPressed() {
        openOrCloseMenu();
    }

    /**
     * This function permit to open the menu, in a static way
     */
    public static void staticOpenOrCloseMenu() {
        if (instance != null) {
            instance.openOrCloseMenu();
        }
    }

    public void openOrCloseMenu() {
        if (drawer.isDrawerOpen(GravityCompat.START)) {
            drawer.closeDrawer(GravityCompat.START);
        } else {
            drawer.openDrawer(GravityCompat.START);
            drawer.setDrawerLockMode(DrawerLayout.LOCK_MODE_LOCKED_OPEN);
        }
    }

    private void showEndGameDialog() {
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
        alertDialogBuilder.setTitle(R.string.app_name);

        // set dialog message
        alertDialogBuilder.setMessage(R.string.do_want_quit).setCancelable(false)
                .setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        endGame();
                    }
                }).setNegativeButton(R.string.no, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                dialog.cancel();
            }
        });

        // create alert dialog
        AlertDialog alertDialog = alertDialogBuilder.create();

        alertDialog.show();
    }

    public static native void toggleFps();

    public static native void endGame();

    protected String[] getArguments() {
        return getIntent().getStringArrayExtra(TAG_COMMAND_LINE);
    }

    /**
     * Used to retrieve the selected game in the browser.
     *
     * @return Full path to game
     */
    public String getProjectPath() {
        return getIntent().getStringExtra(TAG_PROJECT_PATH);
    }


    /**
     * Used by timidity of SDL_mixer to find the timidity folder for the
     * instruments. Invoked via JNI.
     *
     * @return Full path to the timidity.cfg
     */
    public String getTimidityPath() {
        // Log.v("SDL", "getTimidity " +
        // getApplication().getApplicationInfo().dataDir);
        String s = getApplication().getApplicationInfo().dataDir + "/timidity";
        if (new File(s).exists()) {
            return s;
        }

        return SettingsManager.getEasyRPGFolder() + "/timidity"; //Shouldn't be called anymore
    }

    /**
     * Used by the native code to retrieve the RTP directory. Invoked via JNI.
     *
     * @return Full path to the RTP
     */
    public String getRtpPath() {
        String str = SettingsManager.getEasyRPGFolder() + "/rtp";
        // Log.v("SDL", "getRtpPath " + str);
        return str;
    }

    /**
     * Gets the display height in pixel.
     *
     * @return display height in pixel
     */
    public int getScreenHeight() {
        DisplayMetrics displayMetrics = getResources().getDisplayMetrics();
        float screenWidthDp = displayMetrics.heightPixels;
        return (int) screenWidthDp;
    }

    /**
     * Gets the display width in pixel.
     *
     * @return display width in pixel
     */
    public int getScreenWidth() {
        DisplayMetrics displayMetrics = getResources().getDisplayMetrics();
        float screenWidthDp = displayMetrics.widthPixels;
        return (int) screenWidthDp;
    }

    /**
     * Draws all buttons.
     */
    private void addButtons() {
        // Adding the buttons
        for (VirtualButton b : inputLayout.getButtonList()) {
            // We add it, if it's not the case already
            if (b.getParent() != mLayout) {
                if (b.getParent() != null) {
                    ((ViewGroup) b.getParent()).removeAllViews();
                }
                mLayout.addView(b);
            }
        }
        updateButtonsPosition();
    }

    public void updateButtonsPosition() {
        RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(LayoutParams.WRAP_CONTENT,
                LayoutParams.WRAP_CONTENT);

        int screenWidth = getWindowManager().getDefaultDisplay().getWidth();
        int screenHeight = getWindowManager().getDefaultDisplay().getHeight();

        for (VirtualButton b : inputLayout.getButtonList()) {
            Helper.setLayoutPosition(this, b, b.getPosX(), b.getPosY());

            // We have to adjust the position in portrait configuration
            if (getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT) {
                params = (RelativeLayout.LayoutParams) b.getLayoutParams();
                // vertical : use approximatively the second part of the screen
                params.topMargin += (int) (screenHeight / 6);
                // horizontal : use a little gap to avoid button to be out of
                // the screen for button to the right
                if (b.getPosX() > 0.5) {
                    params.leftMargin -= screenWidth / 8;
                }

                b.setLayoutParams(params);
            }
        }
    }

    public void updateScreenPosition() {
        RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(LayoutParams.WRAP_CONTENT,
                LayoutParams.WRAP_CONTENT);
        params.leftMargin = 0;

        if (getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT) {
            params.topMargin = -(getWindowManager().getDefaultDisplay().getHeight() / 2);
        } else {
            params.topMargin = 0;
        }
        surface.setLayoutParams(params);
    }

    /**
     * Called after a screen orientation changement
     */
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        updateScreenPosition();
        updateButtonsPosition();
    }

    /**
     * Called after the activity is being re-displayed
     */
    @Override
    public void onRestart() {
        super.onRestart();
        updateScreenPosition();
        updateButtonsPosition();
    }
}
