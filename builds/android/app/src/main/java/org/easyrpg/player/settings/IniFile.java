package org.easyrpg.player.settings;

import org.ini4j.Wini;

import java.io.File;
import java.io.IOException;

public class IniFile {
    private Wini ini = null;

    public SectionView video;
    public SectionView audio;
    public SectionView engine;

    public IniFile(File iniFile) {
        // Create if missing
        try {
            iniFile.createNewFile();
        } catch (IOException e) {
            throw new RuntimeException("iniFile.createNewFile failed");
        }

        try {
            ini = new Wini(iniFile);
        } catch (IOException e) {
            throw new RuntimeException("new Wini failed");
        }

        video = new SectionView("Video");
        audio = new SectionView("Audio");
        engine = new SectionView("Engine");
    }

    public boolean save() {
        try {
            ini.store(ini.getFile());
            return true;
        } catch (IOException e) {
            return false;
        }
    }

    public boolean has(String section, String key) {
        return ini.get(section, key) != null;
    }

    public int getInteger(String section, String key, int defaultValue) {
        String value = ini.get(section, key);

        if (value == null) {
            return defaultValue;
        } else {
            try {
                return Integer.parseInt(value);
            } catch (NumberFormatException e) {
                return defaultValue;
            }
        }
    }

    public boolean getBoolean(String section, String key, boolean defaultValue) {
        String value = ini.get(section, key);

        if (value == null) {
            return defaultValue;
        } else {
            return value.equals("1") || value.equalsIgnoreCase("true");
        }
    }

    public String getString(String section, String key, String defaultValue) {
        String value = ini.get(section, key);

        if (value == null) {
            return defaultValue;
        } else {
            return value;
        }
    }

    public void set(String section, String key, int value) {
        set(section, key, Integer.toString(value));
    }

    public void set(String section, String key, boolean value) {
        set(section, key, value ? "1" : "0");
    }

    public void set(String section, String key, String value) {
        ini.put(section, key, value);
    }

    class SectionView {
        private final String section;

        public SectionView(String section) {
            this.section = section;
        }

        public boolean has(String key) {
            return IniFile.this.has(section, key);
        }

        public int getInteger(String key, int defaultValue) {
            return IniFile.this.getInteger(section, key, defaultValue);
        }

        public boolean getBoolean(String key, boolean defaultValue) {
            return IniFile.this.getBoolean(section, key, defaultValue);
        }

        public String getString(String key, String defaultValue) {
            return IniFile.this.getString(section, key, defaultValue);
        }

        public void set(String key, int value) {
            IniFile.this.set(section, key, value);
        }

        public void set(String key, boolean value) {
            IniFile.this.set(section, key, value);
        }

        public void set(String key, String value) {
            IniFile.this.set(section, key, value);
        }
    }
}
