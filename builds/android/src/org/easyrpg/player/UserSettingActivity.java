package org.easyrpg.player;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;

/** Settings Activity, also store some setting of the activity to avoid lot of system-call */
public class UserSettingActivity extends PreferenceActivity {
	public static boolean	VIBRATION;
	public static long 		VIBRATION_DURATION = 20; //ms
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.preferences);
	}

	/** Load user preferences */
	public static void updateUserPreferences(Context context){
		SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(context);
		VIBRATION = sharedPref.getBoolean(context.getString(R.string.pref_enable_vibration), true);
	}
}