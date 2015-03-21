package org.easyrpg.player.virtual_buttons;

import java.util.LinkedList;

import org.easyrpg.player.R;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Rect;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

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
			//TODO
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

	/**
	 * Draws all buttons.
	 */
	private void drawButtons() {
		layout.removeAllViews();
		for (VirtualButton b : bList) {
			Utilitary.setLayoutPosition(this, b, b.getPosX(), b.getPosY());
			layout.addView(b);
		}
	}

	public static void dragVirtualButton(VirtualButton v, MotionEvent event) {
		float x, y;
		Rect bound = new Rect(v.getLeft(), v.getTop(), v.getRight(),
				v.getBottom());

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