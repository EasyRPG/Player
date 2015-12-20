package org.easyrpg.player;

import java.io.File;
import java.io.FilenameFilter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;

//TODO : Remove hardcoded string

public class DirectoryChooser {
	AlertDialog.Builder builder;
	AlertDialog dialog;
	ListView listView;
	Context context;

	LinkedList<String> currentDirPath = new LinkedList<String>();
	List<String> dir_list;

	public DirectoryChooser(final Context context, String path) {
		this.context = context;
		this.builder = new AlertDialog.Builder(context);
		this.listView = new ListView(context);

		builder.setTitle("Select a directory");
		builder.setPositiveButton("Ok", null);
		builder.setNeutralButton("Create a directory", null);
		builder.setView(listView);

		// Current directory
		if(path != null){
			currentDirPath.addAll(Arrays.asList(path.split("/")));
		}
		
		//Display the folder's list first
		displayItemList();
		dialog = builder.create();


		// Set up action to button (after the dialog creation to avoid automatic closing)
		// (Android "logic")
		dialog.setOnShowListener(new DialogInterface.OnShowListener() {
			@Override
			public void onShow(DialogInterface d) {
				//OK's button
				Button b = dialog.getButton(AlertDialog.BUTTON_POSITIVE);
				b.setOnClickListener(new View.OnClickListener() {
					@Override
					public void onClick(View view) {
						valid();

						// Dismiss once everything is OK.
						dialog.dismiss();
					}
				});

				//Create Directory's button
				Button b2 = dialog.getButton(AlertDialog.BUTTON_NEUTRAL);
				b2.setOnClickListener(new View.OnClickListener() {
					@Override
					public void onClick(View view) {
						createDirectory();
					}
				});
			}
		});

		dialog.show();
	}

	public void valid() {
		if (context instanceof SettingsActivity) {
			((SettingsActivity) context).changeDirectory(dirListToPath(currentDirPath));
		}
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

	/** Returns the folder's list of path */
	public List<String> listDir(String path) {
		List<String> dirList = new ArrayList<String>();
		
		// Add the ".." item if we're not at the root
		if (!path.equals("/")) {
			dirList.add("..");
		}

		// List the directories in "path"
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
		// Retrieve the folder's list to display
		dir_list = listDir(dirListToPath(currentDirPath));

		listView.setAdapter(new ArrayAdapter<String>(context, android.R.layout.simple_list_item_1, dir_list));

		// When clicking on an item, the list display the new path
		listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
			
			public void onItemClick(AdapterView<?> arg0, View view, int pos, long id) {
				String dirName = dir_list.get(pos);
				
				// Updating the current directory
				if (dirName.equals("..")) {
					currentDirPath.removeLast();
				}
				
				else {
					String newPath = dirListToPath(currentDirPath) + "/" + dir_list.get(pos);
					
					if (isReadable(newPath)) {
						currentDirPath.add(dir_list.get(pos));
					}
					else {
						Toast.makeText(context, newPath + " is not accessible", Toast.LENGTH_SHORT).show();
					}
				}
				
				displayItemList();
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
}
