package org.easyrpg.player.button_mapping;

import android.content.Context;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

public class InputLayout {

    private List<VirtualButton> buttonList = new ArrayList<>();
    private Orientation orientation;
    private Context context;

    public enum Orientation { ORIENTATION_HORIZONTAL, ORIENTATION_VERTICAL }

    public InputLayout(Context context, Orientation orientation) {
        this.context = context;
        this.orientation = orientation;
    }

    public InputLayout(LinkedList<VirtualButton> buttonList) {
        this.buttonList.addAll(buttonList);
    }

    public void add(VirtualButton v) {
        buttonList.add(v);
    }

    public static InputLayout parse(Context context, Orientation orientation, String inputLayoutString) {
        InputLayout inputLayout = new InputLayout(context, orientation);

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
                    inputLayout.add(new VirtualCross(context, posX, posY, size));
                } else if (keyCode == MenuButton.MENU_BUTTON_KEY) {
                    inputLayout.add(new MenuButton(context, posX, posY, size));
                } else {
                    inputLayout.add(VirtualButton.Create(context, keyCode, posX, posY, size));
                }
            }
            return inputLayout;
        } catch (Exception e) {
            if(orientation == Orientation.ORIENTATION_HORIZONTAL) {
                return getDefaultInputLayoutHorizontal(context);
            } else {
                return getDefaultInputLayoutVertical(context);
            }
        }
    }

    @Override
    public String toString() {
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
                return getDefaultInputLayoutHorizontal(context).toString();
            } else {
                return getDefaultInputLayoutVertical(context).toString();
            }
        }
    }

    private static LinkedList<VirtualButton> getDefaultHorizontalButtonList(Context context) {
        LinkedList<VirtualButton> l = new LinkedList<>();
        l.add(new MenuButton(context, 0.01, 0.01, 90));
        l.add(VirtualButton.Create(context, VirtualButton.KEY_FAST_FORWARD, 0.01, 0.2, 90));
        l.add(new VirtualCross(context, 0.01, 0.5, 100));
        l.add(VirtualButton.Create(context, VirtualButton.ENTER, 0.80, 0.7, 100));
        l.add(VirtualButton.Create(context, VirtualButton.CANCEL, 0.90, 0.6, 100));

        return l;
    }

    private static LinkedList<VirtualButton> getDefaultVerticalButtonList(Context context) {
        LinkedList<VirtualButton> l = new LinkedList<>();
        l.add(new MenuButton(context, 0.01, 0.5, 100));
        l.add(VirtualButton.Create(context, VirtualButton.KEY_FAST_FORWARD, 0.01, 0.6, 100));
        l.add(new VirtualCross(context, 0.05, 0.7, 100));
        l.add(VirtualButton.Create(context, VirtualButton.ENTER, 0.65, 0.8, 100));
        l.add(VirtualButton.Create(context, VirtualButton.CANCEL, 0.75, 0.7, 100));

        return l;
    }

    public static InputLayout getDefaultInputLayoutHorizontal(Context context) {
        return new InputLayout(getDefaultHorizontalButtonList(context));
    }

    public static InputLayout getDefaultInputLayoutVertical(Context context) {
        return new InputLayout(getDefaultVerticalButtonList(context));
    }

    public List<VirtualButton> getButtonList() {
        return buttonList;
    }

    public void setButtonList(List<VirtualButton> buttonList) {
        this.buttonList = buttonList;
    }
}
