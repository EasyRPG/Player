package org.easyrpg.player.game_browser;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.channels.FileChannel;
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
			String testFilename = f.getPath() + "/.EASYRPG_WRITE_TEST";
			try {
				w = new FileWriter(testFilename);
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
			
			File testFile = new File(testFilename);
			if (testFile.exists()) {
				// Does not throw
				testFile.delete();
			}
		} else {
			boolean deleteAfter = f.exists();
			try {
				FileWriter w = new FileWriter(f, true);
				w.close();
			} catch (IOException e) {
				return false;
			}
			
			if (deleteAfter) {
				f.delete();
			}
		}
		
		return true;
	}
	
	// https://stackoverflow.com/q/106770/
	public static void copyFile(File sourceFile, File destFile) throws IOException {
	    if(!destFile.exists()) {
	        destFile.createNewFile();
	    }

	    FileChannel source = null;
	    FileChannel destination = null;

	    try {
	        source = new FileInputStream(sourceFile).getChannel();
	        destination = new FileOutputStream(destFile).getChannel();
	        destination.transferFrom(source, 0, source.size());
	    }
	    finally {
	        if(source != null) {
	            source.close();
	        }
	        if(destination != null) {
	            destination.close();
	        }
	    }
	}
	
	private static boolean saveDirectoryContainsSave(ProjectInformation project) {
		if (project.getPath().equals(project.getSavePath())) {
			// Doesn't matter because this is used for the copying logic to the save directory
			return true;
		}
		
		File[] files = getSavegames(new File(project.getSavePath()));
		return files.length > 0;
	}
	
	private static void copySavesFromGameDirectoryToSaveDirectory(ProjectInformation project) {
		if (project.getPath().equals(project.getSavePath())) {
			return;
		}
		
		File[] files = getSavegames(new File(project.getPath()));
		for (final File fileEntry : files) {
        	try {
        		copyFile(fileEntry, new File(project.getSavePath() + "/" + fileEntry.getName()));
        	} catch (IOException e) {
        	}
	    }
	}
	
	public static File[] getSavegames(File folder) {
		File[] files = folder.listFiles();
		ArrayList<File> saveFiles = new ArrayList<File>();
		if (files != null) {
			for (final File fileEntry : files) {
		        if (fileEntry.isFile()) {
		            if (fileEntry.getName().toLowerCase().endsWith(".lsd")) {
		            	saveFiles.add(fileEntry);
		            }
		        }
		    }
		}
		return saveFiles.toArray(new File[saveFiles.size()]);
	}
	
	public static void launchGame(Context context, ProjectInformation project) {
		// Prepare savegames, copy them to the save directory on launch to prevent unwanted side effects
		// e.g. games copied from PC with savegames, or from internal storage.
		if (!saveDirectoryContainsSave(project)) {
			copySavesFromGameDirectoryToSaveDirectory(project);
		}
		
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
			
			// Disable audio depending on user preferences
			if(!SettingsActivity.AUDIO_ENABLED){
				args.add("--disable-audio");
			}

			intent.putExtra(EasyRpgPlayerActivity.TAG_SAVE_PATH, project.getSavePath());
			intent.putExtra(EasyRpgPlayerActivity.TAG_PROJECT_PATH, path);
			intent.putExtra(EasyRpgPlayerActivity.TAG_COMMAND_LINE, args.toArray(new String[args.size()]));
			context.startActivity(intent);
		} else {
			String msg = context.getString(R.string.not_valid_game).replace("$PATH", project.getTitle());
			Toast.makeText(context, msg, Toast.LENGTH_LONG).show();
		}
	}
}
