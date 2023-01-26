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

        video = new SectionView("video");
        audio = new SectionView("audio");
        engine = new SectionView("engine");
    }

    public boolean Save() {
        try {
            ini.store(ini.getFile());
            return true;
        } catch (IOException e) {
            return false;
        }
    }

    public boolean Has(String section, String key) {
        return ini.get(section, key) != null;
    }

    public int GetInteger(String section, String key, int defaultValue) {
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

    public boolean GetBoolean(String section, String key, boolean defaultValue) {
        String value = ini.get(section, key);

        if (value == null) {
            return defaultValue;
        } else {
            return value.equals("1") || value.equalsIgnoreCase("true");
        }
    }

    public String GetString(String section, String key, String defaultValue) {
        String value = ini.get(section, key);

        if (value == null) {
            return defaultValue;
        } else {
            return value;
        }
    }

    public void Set(String section, String key, int value) {
        Set(section, key, Integer.toString(value));
    }

    public void Set(String section, String key, boolean value) {
        Set(section, key, value ? "1" : "0");
    }

    public void Set(String section, String key, String value) {
        ini.put(section, key, value);
    }

    class SectionView {
        private final String section;

        public SectionView(String section) {
            this.section = section;
        }

        public boolean Has(String key) {
            return IniFile.this.Has(section, key);
        }

        public int GetInteger(String key, int defaultValue) {
            return IniFile.this.GetInteger(section, key, defaultValue);
        }

        public boolean GetBoolean(String key, boolean defaultValue) {
            return IniFile.this.GetBoolean(section, key, defaultValue);
        }

        public String GetString(String key, String defaultValue) {
            return IniFile.this.GetString(section, key, defaultValue);
        }

        public void Set(String key, int value) {
            IniFile.this.Set(section, key, value);
        }

        public void Set(String key, boolean value) {
            IniFile.this.Set(section, key, value);
        }

        public void Set(String key, String value) {
            IniFile.this.Set(section, key, value);
        }
    }
}
