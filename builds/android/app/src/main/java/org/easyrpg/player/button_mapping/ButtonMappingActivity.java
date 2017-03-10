package org.easyrpg.player.button_mapping;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.support.design.widget.NavigationView;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.RelativeLayout;

import org.easyrpg.player.Helper;
import org.easyrpg.player.R;
import org.easyrpg.player.button_mapping.ButtonMappingManager.InputLayout;

import java.util.LinkedList;
import java.util.List;

public class ButtonMappingActivity extends Activity implements NavigationView.OnNavigationItemSelectedListener {
    DrawerLayout drawer;
    ViewGroup layoutManager;
    List<VirtualButton> layoutList;
    ButtonMappingManager buttonMappingManager;
    InputLayout inputLayout;

    public static final String TAG_ID = "id";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.button_mapping_activity);

        // Menu configuration
        this.drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
        drawer.setDrawerLockMode(DrawerLayout.LOCK_MODE_LOCKED_CLOSED);
        NavigationView navigationView = (NavigationView) findViewById(R.id.nav_view);
        navigationView.setNavigationItemSelectedListener(this);

        // Hide the status bar
        hideStatusBar();

        layoutManager = (RelativeLayout) findViewById(R.id.button_mapping_activity_layout);

        //Retrive the InputLayout to work with
        Intent intent = getIntent();
        int id = intent.getIntExtra(TAG_ID, 0);
        buttonMappingManager = ButtonMappingManager.getInstance(this);
        inputLayout = buttonMappingManager.getLayoutById(id);

        //We does a copy of the inputLayout's button list
        layoutList = new LinkedList<VirtualButton>();
        VirtualButton vb = null;
        for (VirtualButton b : inputLayout.getButtonList()) {
            if (b instanceof VirtualCross) {
                vb = new VirtualCross(this, b.getPosX(), b.getPosY(), b.getSize());
            } else if (b.keyCode > 0) {
                vb = VirtualButton.Create(this, b.getKeyCode(), b.getPosX(), b.getPosY(), b.getSize());
            } else if (b.keyCode == MenuButton.MENU_BUTTON_KEY) {
                vb = new MenuButton(this, b.getPosX(), b.getPosY(), b.getSize());
            }
            vb.setDebug_mode(true);
            layoutList.add(vb);
        }
        drawButtons();
    }

    @Override
    public boolean onNavigationItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.button_mapping_menu_add_button:
                showSupportedButton();
                openOrCloseMenu();
                break;
            case R.id.button_mapping_menu_reset:
                layoutList = InputLayout.getDefaultInputLayout(this).getButtonList();
                drawButtons();
                openOrCloseMenu();
                break;
            case R.id.button_mapping_menu_exit_without_saving:
                this.finish();
                break;
            case R.id.button_mapping_menu_save_and_quit:
                save();
                this.finish();
                break;
            default:
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
        for (VirtualButton b : layoutList) {
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
        buttonMappingManager.save();
    }

    public void showSupportedButton() {
        Context ctx = getApplicationContext();
        final CharSequence[] items = {
                ctx.getString(R.string.key_enter),
                ctx.getString(R.string.key_cancel),
                ctx.getString(R.string.key_shift),
                "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "+", "-", "*", "/",
                ctx.getString(R.string.menu),
                ctx.getString(R.string.key_fast_forward)
        };
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(getResources().getString(R.string.add_a_button));
        builder.setItems(items, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int item) {
                //Toast.makeText(getApplicationContext(), items[item], Toast.LENGTH_SHORT).show();
                addAButton(items[item].toString());
            }
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
            // API11: KeyEvent.KEYCODE_NUMPAD_ADD
            keyCode = 157;
        } else if (s.equals("-")) {
            // API11: KeyEvent.KEYCODE_NUMPAD_SUBTRACT
            keyCode = 156;
        } else if (s.equals("*")) {
            // API11: KeyEvent.KEYCODE_NUMPAD_MULTIPLY
            keyCode = 155;
        } else if (s.equals("/")) {
            // API11: KeyEvent.KEYCODE_NUMPAD_DIVIDE
            keyCode = 154;
        } else if (s.equals(ctx.getString(R.string.menu))) {
            keyCode = MenuButton.MENU_BUTTON_KEY;
        } else if (s.equals(ctx.getString(R.string.key_fast_forward))) {
            keyCode = KeyEvent.KEYCODE_F;
        }

        VirtualButton vb = null;
        if (keyCode > 0) {
            vb = VirtualButton.Create(this, keyCode, 0.5, 0.5, 100);
        } else if (keyCode == MenuButton.MENU_BUTTON_KEY){
            vb = new MenuButton(this, 0.5, 0.5, 100);
        }
        vb.setDebug_mode(true);
        layoutList.add(vb);
        drawButtons();
    }

    /**
     * Draws all buttons.
     */
    private void drawButtons() {
        layoutManager.removeAllViews();
        for (VirtualButton b : layoutList) {
            b.setDebug_mode(true);
            Helper.setLayoutPosition(this, b, b.getPosX(), b.getPosY());
            layoutManager.addView(b);
        }
    }

    public static void dragVirtualButton(VirtualButton v, MotionEvent event) {
        float x, y;

        int action = event.getActionMasked();

        switch (action) {
            case (MotionEvent.ACTION_DOWN):
            case (MotionEvent.ACTION_MOVE):
                // Calculation of the new view position
                x = (v.getLeft() + event.getX() - v.getWidth() / 2)
                        / v.getResources().getDisplayMetrics().widthPixels;
                y = (v.getTop() + event.getY() - v.getHeight() / 2)
                        / v.getResources().getDisplayMetrics().heightPixels;

                Helper.setLayoutPosition((Activity) v.getContext(), v, x, y);

                v.setPosX(x);
                v.setPosY(y);

                return;
            default:
        }
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
}
