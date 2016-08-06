package org.easyrpg.player;

import java.io.File;
import java.io.FilenameFilter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;


public class DirectoryChooser {
    private static String selected_path;

    AlertDialog.Builder builder;
    AlertDialog dialog;
    ListView listView;
    Context context;

    LinkedList<String> currentDirPath = new LinkedList<String>();
    List<String> dir_list;


    public DirectoryChooser(final Context context, String path, final Runnable runnable) {
        this.context = context;
        this.builder = new AlertDialog.Builder(context);
        this.listView = new ListView(context);

        builder.setPositiveButton(R.string.ok, null);
        builder.setNeutralButton(R.string.quick_access, null);
        builder.setView(listView);

        // Current directory
        if (path != null) {
            currentDirPath.addAll(Arrays.asList(path.split("/")));
        }

        //Display the folder's list first
        dialog = builder.create();
        displayItemList();

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
                        selected_path = dirListToPath(currentDirPath);
                        runnable.run();

                        // Dismiss once everything is OK.
                        dialog.dismiss();
                    }
                });

                //Create Quick access button
                Button b2 = dialog.getButton(AlertDialog.BUTTON_NEUTRAL);
                b2.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        quickAccess();
                    }
                });
            }
        });

        dialog.show();
    }

    public void quickAccess() {
        final ListView lv = new ListView(context);

        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder
                .setTitle(R.string.quick_access)
                .setView(lv)
                .setCancelable(true);

        final AlertDialog quickDialog = builder.create();

        final Map<String, File> allLocations = ExternalStorage.getAllStorageLocations();
        final String[] keySet = allLocations.keySet().toArray(new String[allLocations.size()]);
        lv.setAdapter(new ArrayAdapter<String>(context, android.R.layout.simple_list_item_1, keySet));

        // When clicking on an item, the list display the new path
        lv.setOnItemClickListener(new AdapterView.OnItemClickListener() {

            public void onItemClick(AdapterView<?> arg0, View view, int pos, long id) {
                String newPath = allLocations.get(keySet[pos]).getAbsolutePath();

                if (isReadable(newPath)) {
                    currentDirPath.clear();
                    currentDirPath.addAll(Arrays.asList(newPath.split("/")));

                    displayItemList();
                } else {
                    String msg = context.getString(R.string.path_not_readable).replace("$PATH", newPath);
                    Toast.makeText(context, msg, Toast.LENGTH_SHORT).show();
                }

                quickDialog.dismiss();
            }
        });

        quickDialog.show();
    }

    /**
     * Returns the folder's list of path
     */
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

        if (directories != null) {
            Arrays.sort(directories);
            dirList.addAll(new ArrayList<String>(Arrays.asList(directories)));
        }

        return dirList;
    }

    public void displayItemList() {
        // Retrieve the folder's list to display
        String path = dirListToPath(currentDirPath);
        dir_list = listDir(path);
        dialog.setTitle(path);

        listView.setAdapter(new ArrayAdapter<String>(context, android.R.layout.simple_list_item_1, dir_list));

        // When clicking on an item, the list display the new path
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {

            public void onItemClick(AdapterView<?> arg0, View view, int pos, long id) {
                String dirName = dir_list.get(pos);

                // Updating the current directory
                if (dirName.equals("..")) {
                    currentDirPath.removeLast();
                } else {
                    String newPath = dirListToPath(currentDirPath) + "/" + dir_list.get(pos);

                    if (isReadable(newPath)) {
                        currentDirPath.add(dir_list.get(pos));
                    } else {
                        String msg = context.getString(R.string.path_not_readable).replace("$PATH", newPath);
                        Toast.makeText(context, msg, Toast.LENGTH_SHORT).show();
                    }
                }

                displayItemList();
            }
        });
    }

    public static String dirListToPath(LinkedList<String> l) {
        if (l.size() <= 1)
            return "/";

        StringBuilder s = new StringBuilder();
        for (int i = 0; i < l.size(); i++) {
            if (i > 0) {
                s.append("/");
            }
            s.append(l.get(i));
        }

        return s.toString();
    }

    public boolean isReadable(String path) {
        File file = new File(path);
        boolean canRead = file.canRead();
        if (!canRead) {
            Log.e("isReadable", "Current context not allowed to read : " + path);
        }
        return canRead;
    }

    public static String getSelectedPath() {
        return selected_path;
    }
}
