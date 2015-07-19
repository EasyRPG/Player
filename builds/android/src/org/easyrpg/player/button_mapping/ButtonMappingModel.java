package org.easyrpg.player.button_mapping;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.PrintWriter;
import java.util.LinkedList;

import android.content.Context;
import android.os.Environment;
import android.util.Log;
import android.view.KeyEvent;

public class ButtonMappingModel {
	LinkedList<VirtualButton> keyList;
	
	public ButtonMappingModel(){
		this.keyList = new LinkedList<VirtualButton>();
	}

	public void add(VirtualButton v){
		keyList.add(v);
	}
	
	public static LinkedList<VirtualButton> readButtonMappingFile(Context context, String path){
		ButtonMappingModel b = new ButtonMappingModel();
		BufferedReader bf;
		
		//TODO: lecture fichier
		try{
			bf = new BufferedReader(new FileReader(path));
			Log.i("IOSucces", "Mapping Button File opened with success.");
			String tmp;
			
			while((tmp = bf.readLine()) != null && tmp.length() > 4){
				String[] t = tmp.split(":");
				VirtualButton v;
				
				//Type of button
				int keyCode = Integer.valueOf(t[0]);
				
				//Char of the button (if there is one)
				char charButton = t[1].length() > 0 ? t[1].charAt(0) : ' ';
				
				//PosX
				double posX = Double.valueOf(t[2]);
				
				//PosY
				double posY = Double.valueOf(t[3]);
				
				if(keyCode == VirtualCross.KeyCode){ // -1 is the fake keycode for virtual cross
					v = new VirtualCross(context, posX, posY);
				}else{
					v = new VirtualButton(context, keyCode, charButton, posX, posY);
				}
				
				b.add(v);
			}
			Log.i("Player", "Mapping Button File read with success.");
			bf.close();
		}catch(Exception e){
			Log.e("IOError", "Error reading the button mapping file, loading the default one.");
			return getDefaultButtonMapping(context);
		}
		
		return b.keyList;
	}
	
	public static LinkedList<VirtualButton> readDefaultButtonMappingFile(Context context){
		String button_mapping_path = Environment.getExternalStorageDirectory().getPath() + "/easyrpg/mapping_path";
		return readButtonMappingFile(context, button_mapping_path);
	}
	
	/** Return the default button mapping model : one cross, two buttons */
	public static LinkedList<VirtualButton> getDefaultButtonMapping(Context context){
		ButtonMappingModel b = new ButtonMappingModel();
		
		b.add(new VirtualCross(context, 0.0, 0.5));
		b.add(new VirtualButton(context, KeyEvent.KEYCODE_SPACE, 'A', 0.80, 0.7));
		b.add(new VirtualButton(context, KeyEvent.KEYCODE_B, 'B', 0.90, 0.6));

		return b.keyList;
	}
	
	public static void writeButtonMappingFile(LinkedList<VirtualButton> l){
		String button_mapping_path = Environment.getExternalStorageDirectory().getPath() + "/easyrpg/mapping_path";
		PrintWriter pw;
		
		try{
			pw = new PrintWriter(new File(button_mapping_path));
			Log.i("Player", "Mapping Button File opened with success.");
			
			for(VirtualButton v : l){
				pw.printf("%s:%s:%s:%s\n", v.getKeyCode(), v.getCharButton(), v.getPosX(), v.getPosY());
			}
			pw.flush();
			Log.i("Player", "Mapping Button File writed with success.");
			pw.close();
		}catch(Exception e){
			Log.e("IOError", "Error writing the button mapping file.");
		}
	}
}
