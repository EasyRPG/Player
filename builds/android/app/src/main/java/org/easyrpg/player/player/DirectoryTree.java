package org.easyrpg.player.player;

import java.util.ArrayList;

public class DirectoryTree {
    public String[] names;
    public boolean[] types;

    public DirectoryTree(ArrayList<String> files, ArrayList<Boolean> is_dir) {
        if (files.size() != is_dir.size()) {
            throw new RuntimeException("Size mismatch");
        }

        names = new String[files.size()];
        types = new boolean[is_dir.size()];

        files.toArray(names);
        for (int i = 0; i < is_dir.size(); ++i) {
            types[i] = is_dir.get(i);
        }
    }
}
