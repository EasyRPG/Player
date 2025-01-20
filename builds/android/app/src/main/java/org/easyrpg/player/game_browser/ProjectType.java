package org.easyrpg.player.game_browser;

import android.content.Context;

import org.easyrpg.player.R;

public enum ProjectType {
    UNKNOWN("Unknown"),
    SUPPORTED("Supported"),
    RPG_MAKER_XP("RPG Maker XP"),
    RPG_MAKER_VX("RPG Maker VX"),
    RPG_MAKER_VX_ACE("RPG Maker VX Ace"),
    RPG_MAKER_MV_MZ("RPG Maker MV/MZ"),
    WOLF_RPG_EDITOR("Wolf RPG Editor"),
    ENCRYPTED_2K3_MANIACS("Encrypted 2k3 (Maniacs Patch)"),
    RPG_MAKER_95("RPG Maker 95"),
    SIM_RPG_MAKER_95("Sim RPG Maker 95");

    private final String label;

    ProjectType(String label) {
        this.label = label;
    }

    public String getLabel() {
        return this.label;
    }

    public static ProjectType getProjectType(int i) {
        return ProjectType.values()[i];
    }
}
