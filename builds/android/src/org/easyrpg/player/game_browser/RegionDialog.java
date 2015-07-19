package org.easyrpg.player.game_browser;

import java.io.File;
import java.io.IOException;

import org.easyrpg.player.R;

import android.app.Dialog;
import android.content.Context;
import android.view.View;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.Toast;

/**
 * Used to select the Region (Codepage) of a game.
 */
public class RegionDialog extends Dialog {
	Context context;
	private File iniFile;
	private IniEncodingReader iniReader;
	private RadioGroup rg;

	public RegionDialog(Context context, File iniFile) throws IOException {
		super(context);
		
		this.context = context;
		this.iniFile = iniFile;
		
	    setContentView(R.layout.game_browser_region_dialog);
		setTitle(context.getString(R.string.select_game_region));
		setCancelable(true);
		
		rg = (RadioGroup)findViewById(R.id.rg);
		
		Button cancel = (Button)findViewById(R.id.rd_region_cancel);
		cancel.setOnClickListener(new RegionOnCloseClickListener(this));

		OpenIni();
		
		// Connect after OpenIni to prevent signal by toggle function
		rg.setOnCheckedChangeListener(new RadioGroupOnClickListener(this, context));
	}

	private void OpenIni() throws IOException {
		iniReader = new IniEncodingReader(iniFile);
		String encoding = iniReader.getEncoding();
		RadioButton rb = null;
					
		if (encoding == null) {
			rb = (RadioButton)findViewById(R.id.rd_autodetect);
		} else if (encoding.equals("1252")) {
			rb = (RadioButton)findViewById(R.id.rd_west);
		} else if (encoding.equals("1250")) {
			rb = (RadioButton)findViewById(R.id.rd_east);
		} else if (encoding.equals("932")) {
			rb = (RadioButton)findViewById(R.id.rd_jp);
		} else if (encoding.equals("1251")) {
			rb = (RadioButton)findViewById(R.id.rd_cyrillic);
		} else if (encoding.equals("949")) {
			rb = (RadioButton)findViewById(R.id.rd_korean);
		} else if (encoding.equals("936")) {
			rb = (RadioButton)findViewById(R.id.rd_chinese_simple);
		} else if (encoding.equals("950")) {
			rb = (RadioButton)findViewById(R.id.rd_chinese_traditional);
		} else if (encoding.equals("1253")) {
			rb = (RadioButton)findViewById(R.id.rd_greek);
		} else if (encoding.equals("1254")) {
			rb = (RadioButton)findViewById(R.id.rd_turkish);
		} else if (encoding.equals("1257")) {
			rb = (RadioButton)findViewById(R.id.rd_baltic);
		}
		
		if (rb != null) {
			rb.toggle();
		} else {
			Toast.makeText(getContext(), context.getString(R.string.unknown_region),
					Toast.LENGTH_LONG).show();
		}
	}
	
	private class RadioGroupOnClickListener implements OnCheckedChangeListener {
		Dialog parent;
		Context context;
		
		public RadioGroupOnClickListener(Dialog parent, Context context) {
			this.parent = parent;
			this.context = context;
		}
		
		@Override
		public void onCheckedChanged(RadioGroup group, int checkedId) {
			RadioButton v = (RadioButton)parent.findViewById(checkedId);
			parent.dismiss();
			
			String encoding = null;
			
			if (((RadioButton)findViewById(R.id.rd_autodetect)).isChecked()) {
				encoding = "auto";
			} else if (((RadioButton)findViewById(R.id.rd_west)).isChecked()) {
				encoding = "1252";
			} else if (((RadioButton)findViewById(R.id.rd_east)).isChecked()) {
				encoding = "1250";
			} else if (((RadioButton)findViewById(R.id.rd_jp)).isChecked()) {
				encoding = "932";
			} else if (((RadioButton)findViewById(R.id.rd_cyrillic)).isChecked()) {
				encoding = "1251";
			} else if (((RadioButton)findViewById(R.id.rd_korean)).isChecked()) {
				encoding = "949";
			} else if (((RadioButton)findViewById(R.id.rd_chinese_simple)).isChecked()) {
				encoding = "936";
			} else if (((RadioButton)findViewById(R.id.rd_chinese_traditional)).isChecked()) {
				encoding = "950";
			} else if (((RadioButton)findViewById(R.id.rd_greek)).isChecked()) {
				encoding = "1253";
			} else if (((RadioButton)findViewById(R.id.rd_turkish)).isChecked()) {
				encoding = "1254";
			} else if (((RadioButton)findViewById(R.id.rd_baltic)).isChecked()) {
				encoding = "1257";
			}
			
			if (encoding != null) {
				if (encoding.equals("auto")) {
					iniReader.deleteEncoding();
				} else {
					iniReader.setEncoding(encoding);
				}
				try {
					iniReader.save();
					String msg = context.getString(R.string.region_modification_success).replace("$NAME", v.getText().toString());
					Toast.makeText(context, msg, Toast.LENGTH_LONG).show();
				} catch (IOException e) {
					Toast.makeText(context, context.getString(R.string.region_modification_failed),
							Toast.LENGTH_LONG).show();
				}
			}
		}
	}

	private class RegionOnCloseClickListener implements View.OnClickListener {
		Dialog parent;
		
		public RegionOnCloseClickListener(Dialog parent) {
			this.parent = parent;
		}
		
		@Override
		public void onClick(View v) {
			parent.dismiss();
		}
	}
}