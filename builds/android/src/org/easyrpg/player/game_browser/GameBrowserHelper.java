package org.easyrpg.player.game_browser;

import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.LinkedList;

import org.easyrpg.player.R;
import org.easyrpg.player.SettingsActivity;
import org.easyrpg.player.player.EasyRpgPlayerActivity;

import android.content.Context;
import android.content.Intent;
import android.os.Environment;
import android.widget.Toast;

public class GameBrowserHelper {
	//Files' names
	final static String DATABASE_NAME = "RPG_RT.ldb";
	final static String TREEMAP_NAME = "RPG_RT.lmt";
	final static String INI_FILE = "RPG_RT.ini";

	public static void scanFolder(Context context, File[] list, LinkedList<ProjectInformation> project_list, int depth) {
		if (list != null) {
			for (File file : list) {
				if (!file.getName().startsWith(".")) {
					if (isRpg2kGame(file)) {
						project_list.add(new ProjectInformation(file.getName(), file.getAbsolutePath()));
					} else if (file.isDirectory() && file.canRead() && depth > 0) {
						// Not a RPG2k Game but a directory -> recurse
						scanFolder(context, file.listFiles(), project_list, depth - 1);
					}
				}
			}
		}
	}
	
	public static void scanGame(Context context, LinkedList<ProjectInformation> project_list, LinkedList<String> error_list){
		project_list.clear();
		error_list.clear();
		
		String state = Environment.getExternalStorageState();
		if (!Environment.MEDIA_MOUNTED.equals(state)) {
			error_list.add(context.getString(R.string.no_external_storage));
			return;
		}
		
		// Scanning all the games folders
		boolean first_directory = true;
		for(String path : SettingsActivity.GAMES_DIRECTORIES){
			File dir = new File(path);
			// Verification
			// 1) The folder must exist
			if (!dir.exists() && !dir.mkdirs()) {
				String msg = context.getString(R.string.creating_dir_failed).replace("$PATH", path);
				error_list.add(msg);
				
				continue;
			}
	
			// 2) The folder must be readable
			if (!dir.canRead() || !dir.isDirectory()) {
				String msg = context.getString(R.string.path_not_readable).replace("$PATH", path);
				error_list.add(msg);

				continue;
			}

			// Scan the folder
			File[] list = dir.listFiles();
			// Go 2 directores deep to find games in /easyrpg/games, otherwise only 1
			scanFolder(context, list, project_list, first_directory ? 2 : 1);
			first_directory = false;
		}
		
		// If the scan bring nothing in this folder : we notifiate the user
		if (project_list.size() == 0) {
			String error = context.getString(R.string.no_games_found_and_explanation);
			error_list.add(error);
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
	
	public static Boolean canWrite(File f) {
		if (f.isDirectory()) {
			FileWriter w = null;
			try {
				w = new FileWriter(f.getPath() + "/.EASYRPG_WRITE_TEST");
				// Permissions are checked on open, but it is Android, better be save
				w.write("Android >.<");
			} catch (IOException e) {
				return false;
			} finally {
				try {
					if (w != null) {
						w.close();
					}
				} catch (IOException e) {}
			}
		} else {
			try {
				FileWriter w = new FileWriter(f, true);
				w.close();
			} catch (IOException e) {
				return false;
			}
		}
		
		return true;
	}
	
	public static void launchGame(Context context, ProjectInformation project){
		String path = project.getPath();
		
		// Test again in case somebody messed with the file system
		if (GameBrowserHelper.isRpg2kGame(new File(path))) {
			Intent intent = new Intent(context, EasyRpgPlayerActivity.class);
			ArrayList<String> args = new ArrayList<String>();
			
			// Path of game passed to PlayerActivity via intent "project_path"
			// Command line passed via intent "command_line"
			args.add("--project-path");
			args.add(path);
			
			args.add("--save-path");
			args.add(project.getSavePath());
	
			args.add("--encoding");
			if (project.getEncoding() == null || project.getEncoding().length() == 0) {
				args.add("auto");
			} else {
				args.add(project.getEncoding());
			}
			
			intent.putExtra(EasyRpgPlayerActivity.TAG_PROJECT_PATH, path);
			intent.putExtra(EasyRpgPlayerActivity.TAG_COMMAND_LINE, args.toArray(new String[args.size()]));
			context.startActivity(intent);
		} else {
			String msg = context.getString(R.string.not_valid_game).replace("$PATH", project.getTitle());
			Toast.makeText(context, msg, Toast.LENGTH_LONG).show();
		}
	}
}
