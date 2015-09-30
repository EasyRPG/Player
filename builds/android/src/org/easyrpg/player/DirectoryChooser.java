package org.easyrpg.player;

import java.io.File;
import java.io.FilenameFilter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;

public class DirectoryChooser {
	AlertDialog.Builder builder;
	AlertDialog dialog;
	ListView listView;
	Context context;

	LinkedList<String> currentDirPath;
	ArrayList<String> dir_list;

	public DirectoryChooser(final Context context) {
		this.context = context;
		this.builder = new AlertDialog.Builder(context);
		this.listView = new ListView(context);
		this.currentDirPath = new LinkedList<String>();

		builder.setTitle("Select a directory");
		builder.setPositiveButton("Ok", null);
		builder.setNeutralButton("Create a directory", null);
		builder.setView(listView);

		displayItemList();
		dialog = builder.create();

		// Set up action to button (after the dialog creation to avoid automatic
		// closing)
		dialog.setOnShowListener(new DialogInterface.OnShowListener() {

			@Override
			public void onShow(DialogInterface d) {

				Button b = dialog.getButton(AlertDialog.BUTTON_POSITIVE);
				b.setOnClickListener(new View.OnClickListener() {

					@Override
					public void onClick(View view) {
						// TODO Do something
						valid();

						// Dismiss once everything is OK.
						dialog.dismiss();
					}
				});

				Button b2 = dialog.getButton(AlertDialog.BUTTON_NEUTRAL);
				b2.setOnClickListener(new View.OnClickListener() {

					@Override
					public void onClick(View view) {
						// TODO Do something
						createDirectory();
					}
				});
			}
		});

		dialog.show();
	}

	public ArrayList<String> listDir(String path) {
		ArrayList<String> dirList = new ArrayList<String>();
		if (!path.equals("/")) {
			dirList.add("..");
		}

		File dir = new File(path);
		String[] directories = dir.list(new FilenameFilter() {
			@Override
			public boolean accept(File current, String name) {
				File newFile = new File(current, name);
				return newFile.isDirectory() && !newFile.isHidden();
			}
		});

		dirList.addAll(new ArrayList<String>(Arrays.asList(directories)));

		return dirList;
	}

	public void displayItemList() {
		dir_list = listDir(dirListToPath(currentDirPath));

		listView.setAdapter(new ArrayAdapter<String>(context, android.R.layout.simple_list_item_1, dir_list));
		listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
			public void onItemClick(AdapterView<?> arg0, View view, int pos, long id) {
				String dirName = dir_list.get(pos);
				if (dirName.equals("..")) {
					currentDirPath.removeLast();
					displayItemList();
				} else {
					String newPath = dirListToPath(currentDirPath) + "/" + dir_list.get(pos);
					if (isReadable(newPath)) {
						currentDirPath.add(dir_list.get(pos));
						displayItemList();
					} else {
						Toast.makeText(context, newPath + " is not accessible", Toast.LENGTH_SHORT).show();
					}
				}
			}
		});
	}

	public static String dirListToPath(LinkedList<String> l) {
		String s = new String();
		if (l.isEmpty())
			s += "/";
		for (String st : l) {
			s += "/" + st;
		}

		return s;
	}

	public boolean isReadable(String path) {
		File file = new File(path);
		return file.canRead();
	}

	public void createDirectory() {
		final EditText et = new EditText(context);
		
		AlertDialog.Builder builder = new AlertDialog.Builder(context);
		builder
			.setTitle("Create a directory")
			.setView(et)
			.setPositiveButton("Ok", new OnClickListener() {			
				@Override
				public void onClick(DialogInterface dialog, int which) {
					File f = new File(dirListToPath(currentDirPath) + "/" +  et.getText().toString());
					f.mkdir();
					if(!f.exists()){
						Toast.makeText(context, "Impossible to create the folder", Toast.LENGTH_SHORT).show();
						return;
					}
					displayItemList();
				}
			})
			.setCancelable(true);
		
		builder.show();
	}

	public void valid() {
		if (context instanceof SettingsActivity) {
			((SettingsActivity) context).changeDirectory(dirListToPath(currentDirPath));
		}
	}
}
