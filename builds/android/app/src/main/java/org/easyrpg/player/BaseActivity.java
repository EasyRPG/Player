package org.easyrpg.player;

import android.os.Bundle;
import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;
import org.easyrpg.player.settings.SettingsManager;

/**
 * This activity is used by the GameBrowser and the settings.
 */
public class BaseActivity extends AppCompatActivity {
    public static Boolean libraryLoaded = false;

    private static void loadNativeLibraries() {
        if (!libraryLoaded) {
            try {
                System.loadLibrary("easyrpg_android");
                System.loadLibrary("gamebrowser");
                libraryLoaded = true;
            } catch (UnsatisfiedLinkError e) {
                Log.e("EasyRPG Player", "Couldn't load libgamebrowser: " + e.getMessage());
                throw e;
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        init();
    }

    @Override
    protected void onResume() {
        super.onResume();

        init();
    }

    protected void init() {
        // Retrieve User's preferences
        SettingsManager.init(getApplicationContext());

        loadNativeLibraries();
    }
}
