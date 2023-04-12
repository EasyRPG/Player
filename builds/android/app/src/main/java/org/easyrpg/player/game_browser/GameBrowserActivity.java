package org.easyrpg.player.game_browser;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.Bundle;
import android.provider.DocumentsContract;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBarDrawerToggle;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.core.view.GravityCompat;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.google.android.material.navigation.NavigationView;

import org.easyrpg.player.R;
import org.easyrpg.player.settings.SettingsManager;

import java.util.Collections;
import java.util.List;

public class GameBrowserActivity extends AppCompatActivity
        implements NavigationView.OnNavigationItemSelectedListener {
    public static Boolean libraryLoaded = false;

    private static final int THUMBNAIL_HORIZONTAL_SIZE_DPI = 290;
    private static Game selectedGame;

    private RecyclerView gamesGridRecyclerView;
    private int nbOfGamesPerLine;
    private boolean isScanProcessing;
    private static List<Game> displayedGamesList;

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

        setContentView(R.layout.activity_games_browser);

        // Configure the toolbar
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        // Configure the lateral menu
        DrawerLayout drawer = findViewById(R.id.drawer_layout);
        ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(
                this, drawer, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close);
        drawer.addDrawerListener(toggle);
        toggle.syncState();
        NavigationView navigationView = findViewById(R.id.nav_view);
        navigationView.setNavigationItemSelectedListener(this);
    }

    @Override
    public void onResume() {
        super.onResume();

        scanGamesAndDisplayResult(false);
        GameBrowserHelper.displayHowToMessageOnFirstStartup(this);
    }

    /**
     * Change the grid depending on the orientation
     */
    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        setGamesGridSize();
        scanGamesAndDisplayResult(false);
    }

    @Override
    public void onBackPressed() {
        // Open the lateral menu
        DrawerLayout drawer = findViewById(R.id.drawer_layout);
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

        if (id == R.id.refresh) {
            scanGamesAndDisplayResult(true);
            return true;
        } else if (id == R.id.menu) {
            GameBrowserHelper.openSettingsActivity(this);
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    public boolean onNavigationItemSelected(MenuItem item) {
        // Handle navigation view item clicks here.
        int id = item.getItemId();

        if (id == R.id.nav_help) {
            GameBrowserHelper.displayHowToUseEasyRpgDialog(this);
        } else if (id == R.id.nav_manage) {
            GameBrowserHelper.openSettingsActivity(this);
        } else if (id == R.id.nav_website) {
            Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("https://easyrpg.org"));
            startActivity(browserIntent);
        }

        DrawerLayout drawer = findViewById(R.id.drawer_layout);
        drawer.closeDrawer(GravityCompat.START);
        return true;
    }

    public void scanGamesAndDisplayResult(boolean forceScan) {
        // Verify that a scan isn't processing
        // TODO : Make the use of isScanProcessing synchronized (not really useful)
        if (isScanProcessing){
            return;
        }
        isScanProcessing = true;

        // To limit the number of sys calls, we only scan for games at startup and when the user
        // ask to refresh the games list
        if (forceScan || GameBrowserActivity.displayedGamesList == null) {
            if (forceScan) {
                SettingsManager.clearGamesCache();
            }

            resetGamesList();

            // Empty the games list and display a loading icon
            RelativeLayout content_layout = findViewById(R.id.browser_layout);
            content_layout.removeAllViews();
            getLayoutInflater().inflate(R.layout.loading_panel, content_layout);

            // Start the scan asynchronously
            Activity activity = this;
            new Thread(() -> {
                // Scan games
                GameScanner gameScanner = GameScanner.getInstance(activity);

                // "Only the original thread that created a view hierarchy can touch its views."
                runOnUiThread(() -> {
                    // Populate the list view
                    if (!gameScanner.hasError()) {
                        GameBrowserActivity.displayedGamesList = gameScanner.getGameList();
                        displayGamesList();
                    } else {
                        // Display the errors list
                        content_layout.removeAllViews();
                        getLayoutInflater().inflate(R.layout.browser_error_panel, content_layout);

                        // Set the error text
                        List<String> errorList = gameScanner.getErrorList();
                        StringBuilder errorString = new StringBuilder();
                        for (String error : errorList) {
                            errorString.append(error).append("\n");
                        }
                        TextView errorLayout = findViewById(R.id.error_text);
                        errorLayout.setText(errorString.toString());

                        // Video button
                        findViewById(R.id.watch_video).setOnClickListener(v -> {
                            Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(GameBrowserHelper.VIDEO_URL));
                            startActivity(browserIntent);
                        });
                    }

                    isScanProcessing = false;
                });
            }).start();
        } else {
            displayGamesList();
            isScanProcessing = false;
        }
    }

    /** Only use when the scan is cached */
    public void displayGamesList() {
        RelativeLayout content_layout = findViewById(R.id.browser_layout);

        // Display the game grid
        content_layout.removeAllViews();
        getLayoutInflater().inflate(R.layout.browser_games_grid, content_layout);

        gamesGridRecyclerView = findViewById(R.id.games_grid_recycle_view);
        gamesGridRecyclerView.setHasFixedSize(true);
        setGamesGridSize();

        reorderGameList();
    }

    /** Reorder the displayed game list */
    public void reorderGameList() {
        // Sort the games list : alphabetically ordered, favorite in first
        Collections.sort(displayedGamesList);
        gamesGridRecyclerView.setAdapter(new MyAdapter(this, displayedGamesList, nbOfGamesPerLine));
    }

    /**
     * Set the layout manager depending on the screen orientation
     */
    public void setGamesGridSize() {
        if (gamesGridRecyclerView != null) {
            // Determine the layout template (List or Grid, number of element per line for the grid)
            DisplayMetrics displayMetrics = this.getResources().getDisplayMetrics();
            float dpWidth = displayMetrics.widthPixels / displayMetrics.density;
            this.nbOfGamesPerLine = (int) (dpWidth / THUMBNAIL_HORIZONTAL_SIZE_DPI);

            gamesGridRecyclerView.setLayoutManager(new GridLayoutManager(this, nbOfGamesPerLine));
        }
    }

    public static Game getSelectedGame() {
        return selectedGame;
    }

    public static void resetGamesList() {
        GameBrowserActivity.displayedGamesList = null;
    }

    static class MyAdapter extends RecyclerView.Adapter<MyAdapter.ViewHolder> {
        private final List<Game> gameList;
        private final Activity activity;
        private final int nbOfGamesPerLine;

        public MyAdapter(Activity activity, List<Game> gameList, int nbOfGamesPerLine) {
            this.gameList = gameList;
            this.activity = activity;
            this.nbOfGamesPerLine = nbOfGamesPerLine;
        }

        @Override
        public int getItemCount() {
            return gameList.size();
        }

        // Create new views (invoked by the layout manager)
        @NonNull
        @Override
        public MyAdapter.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            LayoutInflater inflater = LayoutInflater.from(parent.getContext());

            // Inflate the view and fill it
            View v;
            if (this.nbOfGamesPerLine <= 1) {
                v = inflater.inflate(R.layout.browser_game_card_portrait, parent, false);
            } else {
                v = inflater.inflate(R.layout.browser_game_card_landscape, parent, false);
            }
            return new ViewHolder(v);
        }

        // Replace the contents of a view (invoked by the layout manager)
        @Override
        public void onBindViewHolder(final ViewHolder holder, final int position) {
            final Game game = gameList.get(position);

            // Title
            holder.title.setText(game.getTitle());
            holder.title.setOnClickListener(v -> launchGame(position, false));

            // TitleScreen Image
            holder.titleScreen.setImageBitmap(game.getTitleScreen());
            holder.titleScreen.setOnClickListener(v -> launchGame(position, false));

            // Settings Button
            holder.settingsButton.setOnClickListener(v -> {
                String[] choices_list = {
                    activity.getResources().getString(R.string.select_game_region),
                    activity.getResources().getString(R.string.launch_debug_mode)
                };

                AlertDialog.Builder builder = new AlertDialog.Builder(activity);
                builder
                        .setTitle(R.string.settings)
                        .setItems(choices_list, (dialog, which) -> {
                            if (which == 0) {
                                chooseRegion(activity, gameList.get(position));
                            } else if (which == 1) {
                                launchGame(position, true);
                            }
                        });
                builder.show();
            });

            // FavoriteButton
            updateFavoriteButton(holder, game);
            holder.favoriteButton.setOnClickListener(v -> {
                game.setFavorite(!game.isFavorite());
                updateFavoriteButton(holder, game);
                ((GameBrowserActivity)activity).reorderGameList();
            });
        }

        private void launchGame(int position, boolean debugMode) {
            Game selectedGame = gameList.get(position);
            GameBrowserActivity.selectedGame = selectedGame;

            GameBrowserHelper.launchGame(activity, selectedGame, debugMode);
        }

        public void updateFavoriteButton(ViewHolder holder, Game game){
            int buttonImageResource;

            if (game.isFavorite()) {
                if (nbOfGamesPerLine == 1) {
                    buttonImageResource = R.drawable.ic_action_favorite_on_black;
                } else {
                    buttonImageResource = R.drawable.ic_action_favorite_on_white;
                }
            } else {
                if (nbOfGamesPerLine == 1) {
                    buttonImageResource = R.drawable.ic_action_favorite_off_black;
                } else {
                    buttonImageResource = R.drawable.ic_action_favorite_off_white;
                }
            }

            holder.favoriteButton.setImageResource(buttonImageResource);
        }

        public void chooseRegion(final Context context, final Game game) {
            // The list of region choices
            String[] region_array = Encoding.getEncodingDescriptions(context);

            // Retrieve the game's current encoding settings
            Encoding encoding = game.getEncoding();

            // Building the dialog
            AlertDialog.Builder builder = new AlertDialog.Builder(context);
            builder
                .setTitle(R.string.select_game_region)
                .setSingleChoiceItems(region_array, encoding.getIndex(), null)
                .setPositiveButton(R.string.ok, (dialog, id) -> {
                    int selectedPosition = ((AlertDialog) dialog).getListView().getCheckedItemPosition();
                    Encoding selectedEncoding = Encoding.AUTO.getEncodingByIndex(selectedPosition);

                    if (!selectedEncoding.equals(encoding)) {
                        game.setEncoding(selectedEncoding);
                    }
                })
                .setNegativeButton(R.string.cancel, null);
            builder.show();
        }

        public static class ViewHolder extends RecyclerView.ViewHolder {
            public TextView title;
            public ImageView titleScreen;
            public ImageButton settingsButton, favoriteButton;

            public ViewHolder(View v) {
                super(v);
                this.title = v.findViewById(R.id.title);
                this.titleScreen = v.findViewById(R.id.screen);
                this.settingsButton = v.findViewById(R.id.game_browser_thumbnail_option_button);
                this.favoriteButton = v.findViewById(R.id.game_browser_thumbnail_favorite_button);
            }
        }
    }
}
