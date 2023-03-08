package org.easyrpg.player.game_browser;


import android.content.Context;

import org.easyrpg.player.R;

public enum Encoding {
    AUTO(R.string.autodetect, "auto")
    , WEST_EUROPE(R.string.west_europe, "1252")
    , CENTRAL_EASTERN_EUROPE(R.string.east_europe, "1250")
    , JAPANESE(R.string.japan, "932")
    , CYRILLIC(R.string.cyrillic, "1251")
    , KOREAN(R.string.korean, "949")
    , CHINESE_SIMPLE(R.string.chinese_simple, "936")
    , CHINESE_TRADITIONAL(R.string.chinese_traditional, "950")
    , GREEK(R.string.greek, "1253")
    , TURKISH(R.string.turkish, "1254")
    , BALTIC(R.string.baltic, "1257");

    private final int descriptionStringID;
    private final String regionCode;

    Encoding(int descriptionStringID, String regionCode) {
        this.descriptionStringID = descriptionStringID;
        this.regionCode = regionCode;
    }

    public int getIndex(){
        return this.ordinal();
    }

    public Encoding getEncodingByIndex(int i){
        return Encoding.values()[i];
    }

    public static Encoding regionCodeToEnum(String string) {
        String s = string.trim();

        for (Encoding encoding : Encoding.values()) {
            if (encoding.regionCode.equals(s)) {
                return encoding;
            }
        }
        return AUTO;
    }

    public static String[] getEncodingDescriptions(Context context){
        Encoding[] encodings = Encoding.values();
        String[] array = new String[encodings.length];
        for (int i = 0; i < encodings.length; i++) {
            array[i] = context.getString(encodings[i].descriptionStringID);
        }
        return array;
    }

    public String getRegionCode() {
        return regionCode;
    }
}
