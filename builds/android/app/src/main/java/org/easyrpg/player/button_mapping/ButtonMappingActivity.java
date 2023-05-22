package org.easyrpg.player.button_mapping;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.view.WindowManager;

import androidx.core.view.GravityCompat;
import androidx.drawerlayout.widget.DrawerLayout;

import com.google.android.material.navigation.NavigationView;

import org.easyrpg.player.Helper;
import org.easyrpg.player.R;
import org.easyrpg.player.settings.SettingsManager;

import java.util.LinkedList;
import java.util.List;

public class ButtonMappingActivity extends Activity implements NavigationView.OnNavigationItemSelectedListener {
    private DrawerLayout drawer;
    private ViewGroup layoutManager;
    private List<VirtualButton> buttonList;
    private InputLayout inputLayout;
    private InputLayout.Orientation orientation;

    public static final String TAG_ORIENTATION = "orientation";
    public static final int TAG_ORIENTATION_VALUE_HORIZONTAL = 0,TAG_ORIENTATION_VALUE_VERTICAL = 1;

    public static boolean samsungMultitouchWorkaround = false;
    public static int pointerCount = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.button_mapping_activity);

        // Menu configuration
        this.drawer = findViewById(R.id.drawer_layout);
        drawer.setDrawerLockMode(DrawerLayout.LOCK_MODE_LOCKED_CLOSED);
        NavigationView navigationView = findViewById(R.id.nav_view);
        navigationView.setNavigationItemSelectedListener(this);

        // Hide the status bar
        hideStatusBar();

        // Setup the activity depending on the orientation of the inputLayout to modify
        layoutManager = findViewById(R.id.button_mapping_activity_layout);
        if (getIntent().getIntExtra(TAG_ORIENTATION, TAG_ORIENTATION_VALUE_HORIZONTAL) == TAG_ORIENTATION_VALUE_HORIZONTAL) {
            this.inputLayout = SettingsManager.getInputLayoutHorizontal(this);
            this.orientation = InputLayout.Orientation.ORIENTATION_HORIZONTAL;
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        } else {
            this.inputLayout = SettingsManager.getInputLayoutVertical(this);
            this.orientation = InputLayout.Orientation.ORIENTATION_VERTICAL;
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        }

        // Copy the inputLayout's buttons (to be able to reset later)
        buttonList = new LinkedList<>();
        VirtualButton vb = null;
        for (VirtualButton b : inputLayout.getButtonList()) {
            if (b instanceof VirtualCross) {
                vb = new VirtualCross(this, b.getPosX(), b.getPosY(), b.getSize());
            } else if (b.keyCode > 0) {
                vb = VirtualButton.Create(this, b.getKeyCode(), b.getPosX(), b.getPosY(), b.getSize());
            } else if (b.keyCode == MenuButton.MENU_BUTTON_KEY) {
                vb = new MenuButton(this, b.getPosX(), b.getPosY(), b.getSize());
            }
            buttonList.add(vb);

            drawButtons();
        }
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        pointerCount = ev.getPointerCount();
        return super.dispatchTouchEvent(ev);
    }

    /**
     * Called after a screen orientation changement
     */
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        // We draw the button again to match the positions
        drawButtons();
    }

    @Override
    public boolean onNavigationItemSelected(MenuItem item) {
        if (item.getItemId() == R.id.button_mapping_menu_add_button) {
            showSupportedButton();
            openOrCloseMenu();
        } else if (item.getItemId() == R.id.button_mapping_menu_reset) {
            if (orientation == InputLayout.Orientation.ORIENTATION_HORIZONTAL) {
                buttonList = InputLayout.getDefaultInputLayoutHorizontal(this).getButtonList();
            } else {
                buttonList = InputLayout.getDefaultInputLayoutVertical(this).getButtonList();
            }
            drawButtons();
            openOrCloseMenu();
        } else if (item.getItemId() == R.id.button_mapping_menu_exit_without_saving) {
            this.finish();
        } else if (item.getItemId() == R.id.button_mapping_menu_save_and_quit) {
            save();
            this.finish();
        } else {
            return false;
        }
        return true;
    }

    @Override
    public void onBackPressed() {
        openOrCloseMenu();
    }

    public void openOrCloseMenu() {
        if (drawer.isDrawerOpen(GravityCompat.START)) {
            drawer.closeDrawer(GravityCompat.START);
        } else {
            drawer.openDrawer(GravityCompat.START);
        }
    }

    public void hideStatusBar() {
        // Hide the status bar
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
    }

    public void save() {
        //Copy the button from layoutList to the InputLayout
        inputLayout.getButtonList().clear();
        for (VirtualButton b : buttonList) {
            if (b instanceof VirtualCross) {
                inputLayout.getButtonList().add(new VirtualCross(this, b.getPosX(), b.getPosY(), b.getSize()));
            }
            else if (b.keyCode > 0) {
                inputLayout.getButtonList().add(VirtualButton.Create(this, b.getKeyCode(), b.getPosX(), b.getPosY(), b.getSize()));
            } else if (b.keyCode == MenuButton.MENU_BUTTON_KEY) {
                inputLayout.getButtonList().add(new MenuButton(this, b.getPosX(), b.getPosY(), b.getSize()));
            }
        }

        //Save the ButtonMappingModel
        if (orientation == InputLayout.Orientation.ORIENTATION_HORIZONTAL) {
            SettingsManager.setInputLayoutHorizontal(this, inputLayout);
        } else {
            SettingsManager.setInputLayoutVertical(this, inputLayout);
        }
    }

    public void showSupportedButton() {
        Context ctx = getApplicationContext();
        final CharSequence[] items = {
                ctx.getString(R.string.key_enter),
                ctx.getString(R.string.key_cancel),
                ctx.getString(R.string.key_shift),
                "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "+", "-", "*", "/",
                ctx.getString(R.string.menu),
                ctx.getString(R.string.key_fast_forward),
                ctx.getString(R.string.key_debug_menu),
                ctx.getString(R.string.key_debug_walk_through_walls)
        };
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(getResources().getString(R.string.add_a_button));
        builder.setItems(items, (dialog, item) -> {
            //Toast.makeText(getApplicationContext(), items[item], Toast.LENGTH_SHORT).show();
            addAButton(items[item].toString());
        });
        AlertDialog alert = builder.create();
        alert.show();
    }

    public void addAButton(String s) {
        int keyCode = -1;
        Context ctx = getApplicationContext();

        if (s.equals(ctx.getString(R.string.key_enter))) {
            keyCode = KeyEvent.KEYCODE_SPACE;
        } else if (s.equals(ctx.getString(R.string.key_cancel))) {
            keyCode = KeyEvent.KEYCODE_B;
        } else if (s.equals(ctx.getString(R.string.key_shift))) {
            keyCode = KeyEvent.KEYCODE_SHIFT_LEFT;
        } else if (s.equals("0")) {
            keyCode = KeyEvent.KEYCODE_0;
        } else if (s.equals("1")) {
            keyCode = KeyEvent.KEYCODE_1;
        } else if (s.equals("2")) {
            keyCode = KeyEvent.KEYCODE_2;
        } else if (s.equals("3")) {
            keyCode = KeyEvent.KEYCODE_3;
        } else if (s.equals("4")) {
            keyCode = KeyEvent.KEYCODE_4;
        } else if (s.equals("5")) {
            keyCode = KeyEvent.KEYCODE_5;
        } else if (s.equals("6")) {
            keyCode = KeyEvent.KEYCODE_6;
        } else if (s.equals("7")) {
            keyCode = KeyEvent.KEYCODE_7;
        } else if (s.equals("8")) {
            keyCode = KeyEvent.KEYCODE_8;
        } else if (s.equals("9")) {
            keyCode = KeyEvent.KEYCODE_9;
        } else if (s.equals("+")) {
            keyCode = KeyEvent.KEYCODE_NUMPAD_ADD;
        } else if (s.equals("-")) {
            keyCode = KeyEvent.KEYCODE_NUMPAD_SUBTRACT;
        } else if (s.equals("*")) {
            keyCode = KeyEvent.KEYCODE_NUMPAD_MULTIPLY;
        } else if (s.equals("/")) {
            keyCode = KeyEvent.KEYCODE_NUMPAD_DIVIDE;
        } else if (s.equals(ctx.getString(R.string.menu))) {
            keyCode = MenuButton.MENU_BUTTON_KEY;
        } else if (s.equals(ctx.getString(R.string.key_fast_forward))) {
            keyCode = KeyEvent.KEYCODE_F;
        } else if (s.equals(ctx.getString(R.string.key_debug_menu))) {
            keyCode = KeyEvent.KEYCODE_F9;
        } else if (s.equals(ctx.getString(R.string.key_debug_walk_through_walls))) {
            keyCode = KeyEvent.KEYCODE_CTRL_LEFT;
        }

        VirtualButton vb = null;
        if (keyCode > 0) {
            vb = VirtualButton.Create(this, keyCode, 0.5, 0.5, 100);
        } else if (keyCode == MenuButton.MENU_BUTTON_KEY){
            vb = new MenuButton(this, 0.5, 0.5, 100);
        }
        buttonList.add(vb);
        drawButtons();
    }

    /**
     * Draws all buttons.
     */
    private void drawButtons() {
        layoutManager.removeAllViews();
        for (VirtualButton b : buttonList) {
            b.setDebug_mode(true);
            layoutManager.addView(b);
            Helper.setLayoutPosition(this, b, b.getPosX(), b.getPosY());
        }
    }

    public static void dragVirtualButton(VirtualButton v, MotionEvent event) {
        float x, y;

        int action = event.getActionMasked();

        switch (action) {
            case (MotionEvent.ACTION_DOWN):
                // Samsung reports incorrect coordinates when a multitouch across multiple views
                // happens while a game is running. When the bug happens then a touch event will
                // report out-of-bounds coordinates for the initial touch. We use this to detect
                // the bug (see #2915)
                if (pointerCount > 1 && (event.getX() < 0 || event.getY() < 0)) {
                    Log.i("EasyRPG", "Applying Samsung Touch Workaround");
                    samsungMultitouchWorkaround = true;
                }
                // fallthrough
            case (MotionEvent.ACTION_MOVE):
                // Calculation of the new view position
                ViewGroup parentVg = (ViewGroup)v.getParent();

                x = (v.getLeft() + event.getX() - v.getWidth() / 2f)
                        / parentVg.getMeasuredWidth();
                y = (v.getTop() + event.getY() - v.getHeight() / 2f)
                        / parentVg.getMeasuredHeight();

                if (pointerCount > 1 && samsungMultitouchWorkaround) {
                    double scale = Helper.getTouchScale(v.getContext());
                    x /= scale;
                    y /= scale;
                }

                // Rounding (x,y) in order to place the buttons on a grid
                x = Math.round(x*100f) / 100f;
                y = Math.round(y*100f) / 100f;

                Helper.setLayoutPosition((Activity) v.getContext(), v, x, y);

                v.setPosX(x);
                v.setPosY(y);

                return;
            default:
        }
    }
}
