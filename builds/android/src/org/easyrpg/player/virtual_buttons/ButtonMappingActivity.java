package org.easyrpg.player.virtual_buttons;

import java.util.LinkedList;

import org.easyrpg.player.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.widget.RelativeLayout;
import android.widget.Toast;

public class ButtonMappingActivity extends Activity {
	LinkedList<VirtualButton> bList;
	ViewGroup layout;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.buton_mapping_activity);

		layout = (RelativeLayout) findViewById(R.id.button_mapping_activity_layout);
		
		//Get the button list and convert it in debug buttons
		bList = new LinkedList<VirtualButton>();
		LinkedList<VirtualButton> tmp = ButtonMappingModel.readDefaultButtonMappingFile(this);
		for(VirtualButton b : tmp){
			if(b instanceof VirtualCross)
				bList.add(new VirtualCross_Debug(this, (VirtualCross)b));
			else
				bList.add(new VirtualButton_Debug(this, b));
		}
		drawButtons();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.button_mapping_menu, menu);
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case R.id.button_mapping_menu_add_button:
			showSupportedButton();
			return true;
		case R.id.button_mapping_menu_reset:
			bList = ButtonMappingModel.getDefaultButtonMapping(this);
			drawButtons();
			return true;
		case R.id.button_mapping_menu_save:
			ButtonMappingModel.writeButtonMappingFile(bList);
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
	}

	public void showSupportedButton(){
		final CharSequence[] items = {"Enter", "Cancel", "Shift", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "+", "-", "*", "/"};
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
		
		if(s.equals("Enter")){
			keyCode = KeyEvent.KEYCODE_SPACE;
			charButton = 'A';
		}else if(s.equals("Cancel")){
			keyCode = KeyEvent.KEYCODE_B;
			charButton = 'B';
		}else if(s.equals("Shift")){
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
			if(Build.VERSION.SDK_INT >= 11)
				keyCode = KeyEvent.KEYCODE_NUMPAD_ADD;
			else
				Utilitary.showWrongAPIVersion(getApplicationContext());
		}else if(s.equals("-")){
			if(Build.VERSION.SDK_INT >= 11)
				keyCode = KeyEvent.KEYCODE_NUMPAD_SUBTRACT;
			else
				Utilitary.showWrongAPIVersion(getApplicationContext());
		}else if(s.equals("*")){
			if(Build.VERSION.SDK_INT >= 11)
				keyCode = KeyEvent.KEYCODE_NUMPAD_MULTIPLY;
			else
				Utilitary.showWrongAPIVersion(getApplicationContext());
		}else if(s.equals("/")){
			if(Build.VERSION.SDK_INT >= 11)
				keyCode = KeyEvent.KEYCODE_NUMPAD_DIVIDE;
			else
				Utilitary.showWrongAPIVersion(getApplicationContext());
		}
		
		if(charButton == ' '){
			charButton = s.charAt(0);
		}
		
		if(keyCode != -1){
			bList.add(new VirtualButton_Debug(this, keyCode, charButton));
			drawButtons();
		}else{
			Toast.makeText(getApplicationContext(), "Button not supported on this API", Toast.LENGTH_SHORT).show();
		}
	}
	
	/**
	 * Draws all buttons.
	 */
	private void drawButtons() {
		layout.removeAllViews();
		Log.i("Player", bList.size() + " boutons");
		for (VirtualButton b : bList) {
			Utilitary.setLayoutPosition(this, b, b.getPosX(), b.getPosY());
			layout.addView(b);
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

			Utilitary.setLayoutPosition((Activity) v.getContext(), v, x, y);
			
			v.setPosX(x);
			v.setPosY(y);
			
			return;
		default:
			return;
		}
	}

	class VirtualButton_Debug extends VirtualButton {
		float x, y; // Relative position on screen (between 0 and 1)

		public VirtualButton_Debug(Context context, int keyCode, char charButton) {
			super(context, keyCode, charButton);
		}
		public VirtualButton_Debug(Context context, VirtualButton b){
			super(context, b.getKeyCode(), b.getCharButton(), b.getPosX(), b.getPosY());
		}
		
		@Override
		public boolean onTouchEvent(MotionEvent event) {
			ButtonMappingActivity.dragVirtualButton(this, event);
			return true;
		}
	}

	class VirtualCross_Debug extends VirtualCross {
		public VirtualCross_Debug(Context context) {
			super(context);
		}
		public VirtualCross_Debug(Context context, VirtualCross b){
			super(context, b.getPosX(), b.getPosY());
		}

		@Override
		public boolean onTouchEvent(MotionEvent event) {
			ButtonMappingActivity.dragVirtualButton(this, event);
			return true;
		}
	}
}