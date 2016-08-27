package org.easyrpg.player.settings;

enum SettingsEnum {
    VIBRATION_ENABLED("PREF_ENABLE_VIBRATION"),
    VIBRATE_WHEN_SLIDING_DIRECTION("PREF_VIBRATE_WHEN_SLIDING"),
    AUDIO_ENABLED("PREF_AUDIO_ENABLED"),
    LAYOUT_TRANSPARENCY("PREF_LAYOUT_TRANSPARENCY"),
    IGNORE_LAYOUT_SIZE_SETTINGS("PREF_IGNORE_SIZE_SETTINGS"),
    LAYOUT_SIZE("PREF_SIZE_EVERY_BUTTONS"),
    MAIN_DIRECTORY("PREF_DIRECTORY"),
    GAMES_DIRECTORY("PREF_GAME_DIRECTORIES"),
    FORCED_LANDSCAPE("PREF_FORCED_LANDSCAPE");

    private String label;

    SettingsEnum(String s) {
        this.label = s;
    }

    @Override
    public String toString() {
        return label;
    }
}
