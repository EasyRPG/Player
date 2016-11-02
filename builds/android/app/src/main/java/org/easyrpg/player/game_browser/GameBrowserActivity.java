package org.easyrpg.player.game_browser;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Configuration;
import android.os.Bundle;
import android.support.design.widget.NavigationView;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import org.easyrpg.player.R;
import org.easyrpg.player.button_mapping.ButtonMappingManager;
import org.easyrpg.player.settings.SettingsManager;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class GameBrowserActivity extends AppCompatActivity
        implements NavigationView.OnNavigationItemSelectedListener {
    public static Boolean libraryLoaded = false;

    private static final int THUMBNAIL_HORIZONTAL_SIZE_DPI = 200;

    private RecyclerView recyclerView;
    private RecyclerView.LayoutManager layoutManager;
    private int nbOfGamesPerLine;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        if (!libraryLoaded) {
            try {
                System.loadLibrary("gamebrowser");
                libraryLoaded = true;
            } catch (UnsatisfiedLinkError e) { 
                Log.e("EasyRPG Player", "Couldn't load libgamebrowser. XYZ parsing will be unavailable: " + e.getMessage());
            }
        }

        setContentView(R.layout.browser_activity);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        // Configure the lateral menu
        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
        ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(
                this, drawer, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close);
        drawer.setDrawerListener(toggle);
        toggle.syncState();

        NavigationView navigationView = (NavigationView) findViewById(R.id.nav_view);
        navigationView.setNavigationItemSelectedListener(this);
    }

    @Override
    public void onResume() {
        super.onResume();

        // Retrieve user's preferences (for application's folder)
        SettingsManager.init(getApplicationContext());

        // Display the "How to use EasyRPG" on the first startup
        GameBrowserHelper.displayHowToMessageOnFirstStartup(this);

        /// Display the game list
        recyclerView = (RecyclerView) findViewById(R.id.game_browser_api15_recycleview);
        recyclerView.setHasFixedSize(true);
        setLayoutManager(this.getResources().getConfiguration());

        displayGameList(this);
    }

    @Override
    public void onBackPressed() {
        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
        if (drawer.isDrawerOpen(GravityCompat.START)) {
            drawer.closeDrawer(GravityCompat.START);
        } else {
            super.onBackPressed();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.game_browser, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        switch (id) {
            case R.id.refresh:
                displayGameList(this);
                return true;
            case R.id.menu:
                GameBrowserHelper.openSettingsActivity(this);
                return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @SuppressWarnings("StatementWithEmptyBody")
    @Override
    public boolean onNavigationItemSelected(MenuItem item) {
        // Handle navigation view item clicks here.
        int id = item.getItemId();

        if (id == R.id.nav_help) {
            GameBrowserHelper.displayHowToUseEasyRpgDialog(this);
        } else if (id == R.id.nav_manage) {
            GameBrowserHelper.openSettingsActivity(this);
        }

        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
        drawer.closeDrawer(GravityCompat.START);
        return true;
    }

    /**
     * Change the grid depending on the orientation
     */
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        setLayoutManager(newConfig);
        displayGameList(this);
    }

    public void displayGameList(Activity activity) {
        // Scan games
        GameScanner gameScanner = GameScanner.getInstance(activity);

        // Populate the list view
        if (gameScanner.hasError()) {
            recyclerView.setAdapter(new ErrorAdapter(gameScanner.getErrorList(), this));
        } else {
            recyclerView.setAdapter(new MyAdapter(this, gameScanner.getGameList(), nbOfGamesPerLine));
        }
    }

    /**
     * Set the layout manager depending on the screen orientation
     */
    public void setLayoutManager(Configuration configuration) {
        int orientation = configuration.orientation;

        // Determine the layout template (List or Grid, number of element per line for the grid)
        DisplayMetrics displayMetrics = this.getResources().getDisplayMetrics();
        float dpWidth = displayMetrics.widthPixels / displayMetrics.density;
        this.nbOfGamesPerLine = (int)(dpWidth / THUMBNAIL_HORIZONTAL_SIZE_DPI);

        recyclerView.setLayoutManager(new GridLayoutManager(this, nbOfGamesPerLine));
    }

    static class MyAdapter extends RecyclerView.Adapter<MyAdapter.ViewHolder> {
        private List<GameInformation> movieList;
        private Activity activity;
        private int nbOfGamesPerLine;

        IniEncodingReader iniReader;

        public MyAdapter(Activity activity, List<GameInformation> movieList, int nbOfGamesPerLine) {
            this.movieList = movieList;
            this.activity = activity;
            this.nbOfGamesPerLine = nbOfGamesPerLine;
        }

        @Override
        public int getItemCount() {
            return movieList.size();
        }

        // Create new views (invoked by the layout manager)
        @Override
        public MyAdapter.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            LayoutInflater inflater = LayoutInflater.from(parent.getContext());

            // On inflate la vue et on la remplie
            View v;
            if (this.nbOfGamesPerLine <= 1) {
                v = inflater.inflate(R.layout.browser_game_card_vertical, parent, false);
            } else {
                v = inflater.inflate(R.layout.browser_game_card, parent, false);
            }
            return new ViewHolder(v);
        }

        // Replace the contents of a view (invoked by the layout manager)
        @Override
        public void onBindViewHolder(final ViewHolder holder, final int position) {
            final GameInformation movie = movieList.get(position);

            // NB : Un film a forcement un titre, une annee et un poster
            // Titre
            holder.title.setText(movie.getTitle());

            // Poster
            // TODO : Do a caching system for not load
            holder.screen.setImageBitmap(GameScanner.getGameTitleScreen(movie));

            // Define the click action
            holder.screen.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    GameInformation pi = movieList.get(position);

                    if (!pi.read_project_preferences_encoding()) {
                        File iniFile = GameBrowserHelper.getIniOfGame(pi.getGameFolderPath(), false);

                        // Retrieve the current region (to check the correct radio button)
                        if (iniFile != null) {
                            iniReader = null;
                            try {
                                iniReader = new IniEncodingReader(iniFile);
                                String encoding = iniReader.getEncoding();
                                pi.setEncoding(encoding);
                            } catch (IOException e) {
                            }
                        }
                    }

                    GameBrowserHelper.launchGame(activity, pi);
                }
            });

            holder.settingsButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    String[] choices_list = {activity.getResources().getString(R.string.select_game_region), activity.getString(R.string.change_the_layout)};

                    AlertDialog.Builder builder = new AlertDialog.Builder(activity);
                    builder
                            .setTitle(R.string.settings)
                            .setItems(choices_list, new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    switch (which) {
                                        case 0:
                                            chooseRegion(activity, movieList.get(position));
                                            break;
                                        case 1:
                                            chooseLayout(activity, movieList.get(position));
                                            break;
                                        default:
                                            break;
                                    }
                                }
                            });
                    builder.show();
                }
            });
        }

        public void chooseLayout(final Context context, final GameInformation pi) {
            final ButtonMappingManager buttonMappingManager = ButtonMappingManager.getInstance(context);
            String[] layout_name_array = buttonMappingManager.getLayoutsNames();

            //Detect default layout
            pi.getProjectInputLayout(buttonMappingManager);
            int id = -1;
            for (int i = 0; i < buttonMappingManager.getLayoutList().size(); i++) {
                if (buttonMappingManager.getLayoutList().get(i).getId() == pi.getId_input_layout()) {
                    id = i;
                    break;
                }
            }

            final ArrayList<Integer> selected = new ArrayList<Integer>();
            AlertDialog.Builder builder = new AlertDialog.Builder(context);
            builder
                    .setTitle(R.string.choose_layout)
                    .setSingleChoiceItems(layout_name_array, id, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            selected.clear();
                            selected.add(Integer.valueOf(which));
                        }
                    })
                    .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int id) {
                            if (!selected.isEmpty()) {
                                pi.setId_input_layout(buttonMappingManager.getLayoutList().get(selected.get(0)).getId());
                                pi.write_project_preferences();
                            }
                        }
                    })
                    .setNegativeButton(R.string.cancel, null);
            builder.show();
        }

        public void chooseRegion(final Context context, final GameInformation pi) {
            //The list of region choices
            String[] region_array = {
                    context.getString(R.string.autodetect),
                    context.getString(R.string.west_europe),
                    context.getString(R.string.east_europe),
                    context.getString(R.string.japan),
                    context.getString(R.string.cyrillic),
                    context.getString(R.string.korean),
                    context.getString(R.string.chinese_simple),
                    context.getString(R.string.chinese_traditional),
                    context.getString(R.string.greek),
                    context.getString(R.string.turkish),
                    context.getString(R.string.baltic)
            };

            //Retrieve the project's .ini file
            String encoding = null;

            if (!pi.read_project_preferences_encoding()) {
                File iniFile = GameBrowserHelper.getIniOfGame(pi.getGameFolderPath(), false);

                // Retrieve the current region (to check the correct radio button)
                if (iniFile != null) {
                    iniReader = null;
                    try {
                        iniReader = new IniEncodingReader(iniFile);
                        encoding = iniReader.getEncoding();
                    } catch (IOException e) {
                    }
                }
            } else {
                encoding = pi.getEncoding();
            }

            int id = -1;

            if (encoding == null || encoding.equals("auto")) {
                id = 0;
            } else if (encoding.equals("1252")) {
                id = 1;
            } else if (encoding.equals("1250")) {
                id = 2;
            } else if (encoding.equals("932")) {
                id = 3;
            } else if (encoding.equals("1251")) {
                id = 4;
            } else if (encoding.equals("949")) {
                id = 5;
            } else if (encoding.equals("936")) {
                id = 6;
            } else if (encoding.equals("950")) {
                id = 7;
            } else if (encoding.equals("1253")) {
                id = 8;
            } else if (encoding.equals("1254")) {
                id = 9;
            } else if (encoding.equals("1257")) {
                id = 10;
            }

            if (id == -1) {
                Toast.makeText(context, context.getString(R.string.unknown_region), Toast.LENGTH_LONG).show();
            }

            //Building the dialog
            AlertDialog.Builder builder = new AlertDialog.Builder(context);
            builder
                    .setTitle(R.string.select_game_region)
                    .setSingleChoiceItems(region_array, id, null)
                    .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int id) {
                            int selectedPosition = ((AlertDialog) dialog).getListView().getCheckedItemPosition();
                            String encoding = null;

                            if (selectedPosition == 0) {
                                encoding = "auto";
                            } else if (selectedPosition == 1) {
                                encoding = "1252";
                            } else if (selectedPosition == 2) {
                                encoding = "1250";
                            } else if (selectedPosition == 3) {
                                encoding = "932";
                            } else if (selectedPosition == 4) {
                                encoding = "1251";
                            } else if (selectedPosition == 5) {
                                encoding = "949";
                            } else if (selectedPosition == 6) {
                                encoding = "936";
                            } else if (selectedPosition == 7) {
                                encoding = "950";
                            } else if (selectedPosition == 8) {
                                encoding = "1253";
                            } else if (selectedPosition == 9) {
                                encoding = "1254";
                            } else if (selectedPosition == 10) {
                                encoding = "1257";
                            }

                            if (encoding != null) {
                                pi.setEncoding(encoding);

                                pi.write_project_preferences();
                            }
                        }
                    })
                    .setNegativeButton(R.string.cancel, null);
            builder.show();
        }

        public static class ViewHolder extends RecyclerView.ViewHolder {
            public TextView title;
            public ImageView screen;
            public ImageButton settingsButton;

            public ViewHolder(View v) {
                super(v);
                this.title = (TextView) v.findViewById(R.id.title);
                this.screen = (ImageView) v.findViewById(R.id.screen);
                this.settingsButton = (ImageButton) v.findViewById(R.id.game_browser_thumbnail_option_button);
            }
        }
    }

    static class ErrorAdapter extends RecyclerView.Adapter<ErrorAdapter.ErrorViewHolder> {
        List<String> errorList;

        public ErrorAdapter(List<String> errorList, Context context) {
            this.errorList = errorList;
        }

        @Override
        public ErrorViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            View v = LayoutInflater.from(parent.getContext())
                    .inflate(R.layout.browser_error_text, parent, false);
            ErrorViewHolder viewHolder = new ErrorViewHolder(v);
            return viewHolder;
        }

        @Override
        public void onBindViewHolder(ErrorViewHolder holder, int position) {
            holder.text.setText(errorList.get(position).toString());
        }

        @Override
        public int getItemCount() {
            return errorList.size();
        }

        public static class ErrorViewHolder extends RecyclerView.ViewHolder {
            protected TextView text;

            public ErrorViewHolder(View itemView) {
                super(itemView);
                text = (TextView) itemView.findViewById(R.id.error_code);
            }
        }
    }
}
