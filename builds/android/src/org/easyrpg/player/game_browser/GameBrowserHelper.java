package org.easyrpg.player.game_browser;

import java.io.File;
import java.io.IOException;
import java.util.LinkedList;

import org.easyrpg.player.R;
import org.easyrpg.player.player.EasyRpgPlayerActivity;

import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.os.Environment;
import android.widget.Toast;

public class GameBrowserHelper {
	//Files' names
	final static String DATABASE_NAME = "RPG_RT.ldb";
	final static String TREEMAP_NAME = "RPG_RT.lmt";
	final static String INI_FILE = "RPG_RT.ini";
		
	public static void scanGame(Context context, String path, LinkedList<ProjectInformation> project_list, LinkedList<String> error_list){
		project_list.clear();
		error_list.clear();
		
		String state = Environment.getExternalStorageState();
		if (Environment.MEDIA_MOUNTED.equals(state)) {
		
			File dir = new File(path);
			if (!dir.exists() && !dir.mkdirs()) {
				String msg = context.getString(R.string.creating_dir_failed).replace("$PATH", path);
				error_list.add(msg);
			}
	
			if (!dir.canRead() || !dir.isDirectory()) {
				String msg = context.getString(R.string.path_not_readable).replace("$PATH", path);
				error_list.add(msg);
			} else {
				File[] list = dir.listFiles();
				if (list != null) {
					if(list.length == 0){
						error_list.add(context.getString(R.string.no_games_found));
					}else {
						for (File file : list) {
							if (!file.getName().startsWith(".") && isRpg2kGame(file)) {
								project_list.add(new ProjectInformation(file.getName(), file.getAbsolutePath()));
							}
						}
					}
				}
			}
		} else {
			error_list.add(context.getString(R.string.no_external_storage));
		}
	}
	
	/**
	 * Tests if a folder is a RPG2k Game.
	 * (contains DATABASE_NAME and TREEMAP_NAME)
	 * @param dir Directory to test
	 * @return true if RPG2k game
	 */
	public static boolean isRpg2kGame(File dir) {
		if (!dir.isDirectory() || !dir.canRead()) {
			return false;
		}

		boolean databaseFound = false;
		boolean treemapFound = false;

		for (File entry : dir.listFiles()) {
			if (entry.isFile() && entry.canRead()) {
				if (!databaseFound && entry.getName().equalsIgnoreCase(DATABASE_NAME)) {
					databaseFound = true;
				} else if (!treemapFound && entry.getName().equalsIgnoreCase(TREEMAP_NAME)) {
					treemapFound = true;
				}

				if (databaseFound && treemapFound) {
					return true;
				}
			}
		}

		return false;
	}
	
	/**
	 * Returns Ini File of game at index.
	 * Optionally creates it.
	 * 
	 * @param index list index
	 * @param create create ini if not found
	 * @return ini
	 */
	public static File getIniOfGame(String path, boolean create) {
		File dir = new File(path);
		
		if (!dir.isDirectory() || !dir.canRead()) {
			return null;
		}

		for (File entry : dir.listFiles()) {
			if (entry.isFile() && entry.canRead()) {
				if (entry.getName().equalsIgnoreCase(INI_FILE)) {
					return entry;
				}
			}
		}
		
		if (create) {
			File newIni = new File(dir.getAbsolutePath() + "/RPG_RT.ini");
			
			try {
				newIni.createNewFile();
			} catch (IOException e) {
				return null;
			}
			
			return newIni;
		}

		return null;
	}
	
	public static void launchGame(Context context, ProjectInformation project){
		String path = project.getPath();
		
		// Test again in case somebody messed with the file system
		if (GameBrowserHelper.isRpg2kGame(new File(path))) {
			Intent intent = new Intent(context, EasyRpgPlayerActivity.class);
			// Path of game passed to PlayerActivity via intent "project_path"
			intent.putExtra(EasyRpgPlayerActivity.TAG_PROJECT_PATH, path);
			context.startActivity(intent);
		} else {
			String msg = context.getString(R.string.not_valid_game).replace("$PATH", project.getTitle());
			Toast.makeText(context, msg, Toast.LENGTH_LONG).show();
		}
	}
}
