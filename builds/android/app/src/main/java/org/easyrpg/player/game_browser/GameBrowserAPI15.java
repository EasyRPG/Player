package org.easyrpg.player.game_browser;

import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.NavigationView;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.widget.ArrayAdapter;
import android.widget.ListAdapter;
import android.widget.ListView;

import org.easyrpg.player.R;
import org.easyrpg.player.SettingsActivity;

import java.util.LinkedList;

public class GameBrowserAPI15 extends AppCompatActivity
        implements NavigationView.OnNavigationItemSelectedListener {


    ListView list_view;
    private ListAdapter adapter;
    LinkedList<ProjectInformation> project_list = new LinkedList<ProjectInformation>();
    LinkedList<String> error_list = new LinkedList<String>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_game_browser_api15);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                GameBrowserHelper.openSettingsActivity(GameBrowserAPI15.this);
            }
        });

        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
        ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(
                this, drawer, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close);
        drawer.setDrawerListener(toggle);
        toggle.syncState();

        NavigationView navigationView = (NavigationView) findViewById(R.id.nav_view);
        navigationView.setNavigationItemSelectedListener(this);

        /// Display the game list
        list_view = (ListView)findViewById(R.id.game_browser_list_view);
        displayGameList();

        // Display the "How to use EasyRPG" on the first startup
        GameBrowserHelper.displayHowToMessageOnFirstStartup(this);
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
        getMenuInflater().inflate(R.menu.game_browser_api15, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.refresh) {
            displayGameList();
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

        } else if (id == R.id.nav_share) {

        }

        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
        drawer.closeDrawer(GravityCompat.START);
        return true;
    }

    public void displayGameList(){
        //Scan the folder
        SettingsActivity.updateUserPreferences(this);
        GameBrowserHelper.scanGame(this, project_list, error_list);

        // Put the result into the proper adapter
        if (error_list.size() > 0) {
            //If the game list is empty, we use a simplified adapter
            adapter = new ArrayAdapter<String>(this,
                    android.R.layout.simple_list_item_2, android.R.id.text1, error_list);
        } else {
            //If the game list is not empty, we use the proper adapter
            adapter = new GameListAdapter(this, project_list);
        }

        Log.i("Browser", error_list.size() + " games found : " + project_list);

        //Set the view;
        list_view.setAdapter(adapter);
    }
}
