package org.easyrpg.player.button_mapping;

import java.util.LinkedList;

import org.easyrpg.player.Helper;
import org.easyrpg.player.R;
import org.easyrpg.player.button_mapping.ButtonMappingManager.InputLayout;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.widget.RelativeLayout;
import android.widget.Toast;

public class ButtonMappingActivity extends Activity {
	ViewGroup layoutManager;
	LinkedList<VirtualButton> layoutList;
	ButtonMappingManager buttonMappingManager;
	InputLayout inputLayout;
	
	public static final String TAG_ID = "id";

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.button_mapping_activity);

		layoutManager = (RelativeLayout) findViewById(R.id.button_mapping_activity_layout);

		//Retrive the InputLayout to work with
		Intent intent = getIntent();
		int id = intent.getIntExtra(TAG_ID, 0);
		buttonMappingManager = ButtonMappingManager.getButtonMapping(this);
		inputLayout = buttonMappingManager.getLayoutById(this, id);
		
		//We does a copy of the inputLayout's button list
		layoutList = new LinkedList<VirtualButton>();
		for(VirtualButton b : inputLayout.getButton_list()){
			if(b instanceof VirtualCross){
				VirtualCross v = new VirtualCross(this, b.getPosX(), b.getPosY(), b.getSize());
				layoutList.add(v);
			}
			else{
				VirtualButton vb = new VirtualButton(this, b.getKeyCode(), b.getPosX(), b.getPosY(), b.getSize());
				layoutList.add(vb);
			}
		}
		drawButtons();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.button_mapping, menu);
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case R.id.button_mapping_menu_add_button:
			showSupportedButton();
			return true;
		case R.id.button_mapping_menu_reset:
			layoutList = InputLayout.getDefaultInputLayout(this).getButton_list();
			drawButtons();
			return true;
		case R.id.button_mapping_menu_exit_without_saving:
			this.finish();
			return true;
		case R.id.button_mapping_menu_save_and_quit:
			save();
			this.finish();
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
	}

	@Override
	public void onBackPressed(){
		openOptionsMenu ();
	}
	
	/** This function prevents some Samsung's device to not show the option menu
	 */
	@Override
	public void openOptionsMenu() {

	    Configuration config = getResources().getConfiguration();

	    if((config.screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK) 
	            > Configuration.SCREENLAYOUT_SIZE_LARGE) {

	        int originalScreenLayout = config.screenLayout;
	        config.screenLayout = Configuration.SCREENLAYOUT_SIZE_LARGE;
	        super.openOptionsMenu();
	        config.screenLayout = originalScreenLayout;

	    } else {
	        super.openOptionsMenu();
	    }
	}
	
	public void save(){
		//Copy the button from layoutList to the InputLayout
		inputLayout.getButton_list().clear();
		for(VirtualButton b : layoutList){
			if(b instanceof VirtualCross)
				inputLayout.getButton_list().add(new VirtualCross(this, b.getPosX(), b.getPosY(), b.getSize()));
			else
				inputLayout.getButton_list().add(new VirtualButton(this, b.getKeyCode(), b.getPosX(), b.getPosY(), b.getSize()));
		}
		
		//Save the ButtonMappingModel
		ButtonMappingManager.writeButtonMappingFile(this, buttonMappingManager);
	}
	
	public void showSupportedButton(){
		Context ctx = getApplicationContext();
		final CharSequence[] items = {
				ctx.getString(R.string.key_enter),
				ctx.getString(R.string.key_cancel),
				ctx.getString(R.string.key_shift),
				"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "+", "-", "*", "/"};
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
	
	public void addAButton(String s){
		int keyCode = -1;
		char charButton = ' ';
		Context ctx = getApplicationContext();
		
		if(s.equals(ctx.getString(R.string.key_enter))){
			keyCode = KeyEvent.KEYCODE_SPACE;
			charButton = 'A';
		}else if(s.equals(ctx.getString(R.string.key_cancel))){
			keyCode = KeyEvent.KEYCODE_B;
			charButton = 'B';
		}else if(s.equals(ctx.getString(R.string.key_shift))){
			keyCode = KeyEvent.KEYCODE_SHIFT_LEFT;
			charButton = 'S';
		}else if(s.equals("0")){
			keyCode = KeyEvent.KEYCODE_0;
		}else if(s.equals("1")){
			keyCode = KeyEvent.KEYCODE_1;
		}else if(s.equals("2")){
			keyCode = KeyEvent.KEYCODE_2;
		}else if(s.equals("3")){
			keyCode = KeyEvent.KEYCODE_3;
		}else if(s.equals("4")){
			keyCode = KeyEvent.KEYCODE_4;
		}else if(s.equals("5")){
			keyCode = KeyEvent.KEYCODE_5;
		}else if(s.equals("6")){
			keyCode = KeyEvent.KEYCODE_6;
		}else if(s.equals("7")){
			keyCode = KeyEvent.KEYCODE_7;
		}else if(s.equals("8")){
			keyCode = KeyEvent.KEYCODE_8;
		}else if(s.equals("9")){
			keyCode = KeyEvent.KEYCODE_9;
		}else if(s.equals("+")){
			// API11: KeyEvent.KEYCODE_NUMPAD_ADD
			keyCode = 157;
		}else if(s.equals("-")){
			// API11: KeyEvent.KEYCODE_NUMPAD_SUBTRACT
			keyCode = 156;
		}else if(s.equals("*")){
			// API11: KeyEvent.KEYCODE_NUMPAD_MULTIPLY
			keyCode = 155;
		}else if(s.equals("/")){
			// API11: KeyEvent.KEYCODE_NUMPAD_DIVIDE
			keyCode = 154;
		}
		
		if(charButton == ' '){
			charButton = s.charAt(0);
		}
		
		if(keyCode != -1){
			VirtualButton vb = new VirtualButton(this, keyCode, 0.5, 0.5, 100);
			vb.setDebug_mode(true);
			layoutList.add(vb);
			drawButtons();
		}else{
			Toast.makeText(getApplicationContext(), "Button not supported on this API", Toast.LENGTH_SHORT).show();
		}
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
	
	/** Called after a screen orientation changement */
	@Override
	public void onConfigurationChanged(Configuration newConfig) {
	    super.onConfigurationChanged(newConfig);

	    // We draw the button again to match the positions
	    drawButtons();
	}
}