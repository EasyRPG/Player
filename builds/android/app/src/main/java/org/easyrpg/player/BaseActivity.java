package org.easyrpg.player;

import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;
import org.easyrpg.player.settings.SettingsManager;

public class BaseActivity  extends AppCompatActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Retrieve User's preferences
        SettingsManager.init(getApplicationContext());
    }

    @Override
    protected void onResume() {
        super.onResume();

        // Retrieve User's preferences
        SettingsManager.init(getApplicationContext());
    }
}
