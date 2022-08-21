package org.easyrpg.player.button_mapping;

import android.app.Activity;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

public class InputLayout {

    private final List<VirtualButton> buttonList = new ArrayList<>();
    private Orientation orientation;

    public enum Orientation { ORIENTATION_HORIZONTAL, ORIENTATION_VERTICAL }

    public InputLayout(Orientation orientation) {
        this.orientation = orientation;
    }

    public InputLayout(LinkedList<VirtualButton> buttonList) {
        this.buttonList.addAll(buttonList);
    }

    public void add(VirtualButton v) {
        buttonList.add(v);
    }

    public static InputLayout parse(Activity activity, Orientation orientation, String inputLayoutString) {
        InputLayout inputLayout = new InputLayout(orientation);

        try {
            String[] buttonStringList = inputLayoutString.split(";");
            for (String buttonString : buttonStringList) {
                // ButtonString format : "keyCode:size:posX:posY"
                String[] buttonInfo = buttonString.split(":");
                int keyCode = Integer.parseInt(buttonInfo[0]);
                int size = Integer.parseInt(buttonInfo[1]);
                double posX = Double.parseDouble(buttonInfo[2]);
                double posY = Double.parseDouble(buttonInfo[3]);
                if (keyCode == VirtualButton.DPAD) {
                    inputLayout.add(new VirtualCross(activity, posX, posY, size));
                } else if (keyCode == MenuButton.MENU_BUTTON_KEY) {
                    inputLayout.add(new MenuButton(activity, posX, posY, size));
                } else {
                    inputLayout.add(VirtualButton.Create(activity, keyCode, posX, posY, size));
                }
            }
            return inputLayout;
        } catch (Exception e) {
            if(orientation == Orientation.ORIENTATION_HORIZONTAL) {
                return getDefaultInputLayoutHorizontal(activity);
            } else {
                return getDefaultInputLayoutVertical(activity);
            }
        }
    }

    public String toStringForSave(Activity activity) {
        try {
            StringBuilder sb = new StringBuilder();

            for (VirtualButton vb : buttonList) {
                sb.append(vb.getKeyCode()).append(":");
                sb.append(vb.getSize()).append(":");
                sb.append(vb.getPosX()).append(":");
                sb.append(vb.getPosY()).append(":");
                sb.append(";");
            }
            return sb.toString();
        } catch (Exception e) {
            if (orientation == Orientation.ORIENTATION_HORIZONTAL) {
                return getDefaultInputLayoutHorizontal(activity).toString();
            } else {
                return getDefaultInputLayoutVertical(activity).toString();
            }
        }
    }

    private static LinkedList<VirtualButton> getDefaultHorizontalButtonList(Activity activity) {
        LinkedList<VirtualButton> l = new LinkedList<>();
        l.add(new MenuButton(activity, 0.01, 0.01, 90));
        l.add(VirtualButton.Create(activity, VirtualButton.KEY_FAST_FORWARD, 0.9, 0.01, 90));
        l.add(new VirtualCross(activity, 0.01, 0.4, 100));
        l.add(VirtualButton.Create(activity, VirtualButton.ENTER, 0.80, 0.55, 100));
        l.add(VirtualButton.Create(activity, VirtualButton.CANCEL, 0.90, 0.45, 100));

        return l;
    }

    private static LinkedList<VirtualButton> getDefaultVerticalButtonList(Activity activity) {
        LinkedList<VirtualButton> l = new LinkedList<>();
        l.add(new MenuButton(activity, 0.01, 0.5, 90));
        l.add(VirtualButton.Create(activity, VirtualButton.KEY_FAST_FORWARD, 0.70, 0.5, 90));
        l.add(new VirtualCross(activity, 0.05, 0.65, 100));
        l.add(VirtualButton.Create(activity, VirtualButton.ENTER, 0.60, 0.75, 100));
        l.add(VirtualButton.Create(activity, VirtualButton.CANCEL, 0.70, 0.65, 100));

        return l;
    }

    public static InputLayout getDefaultInputLayoutHorizontal(Activity activity) {
        return new InputLayout(getDefaultHorizontalButtonList(activity));
    }

    public static InputLayout getDefaultInputLayoutVertical(Activity activity) {
        return new InputLayout(getDefaultVerticalButtonList(activity));
    }

    public List<VirtualButton> getButtonList() {
        return buttonList;
    }

}
