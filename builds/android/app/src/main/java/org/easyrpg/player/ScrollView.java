package org.easyrpg.player;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Insets;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.util.AttributeSet;
import android.view.View;
import android.view.WindowInsets;

import androidx.annotation.NonNull;

/*
Rendering and layout code based on androidx/drawerlayout/widget/DrawerLayout.java
*/

public class ScrollView extends android.widget.ScrollView {
    private WindowInsets insets = null;
    private Drawable statusBarBackground = null;

    private static final int[] THEME_ATTRS = {
        android.R.attr.colorPrimaryDark
    };

    public ScrollView(Context context) {
        super(context);
        init();
    }

    public ScrollView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }


    public ScrollView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    public ScrollView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        init();
    }

    private void init() {
        if (Build.VERSION.SDK_INT >= 35) {
            setOnApplyWindowInsetsListener(new View.OnApplyWindowInsetsListener() {
                @NonNull
                @Override
                public WindowInsets onApplyWindowInsets(@NonNull View view, @NonNull WindowInsets insets) {
                    final ScrollView thisView = (ScrollView) view;

                    Insets systemBarsInsets = insets.getInsets(WindowInsets.Type.systemBars());
                    thisView.setPadding(
                        systemBarsInsets.left,
                        systemBarsInsets.top,
                        systemBarsInsets.right,
                        systemBarsInsets.bottom
                    );

                    thisView.setChildInsets(insets);

                    return WindowInsets.CONSUMED;
                }
            });


            try (TypedArray a = getContext().obtainStyledAttributes(THEME_ATTRS)) {
                statusBarBackground = a.getDrawable(0);
            }
        }
    }

    public void setChildInsets(WindowInsets insets) {
        this.insets = insets;
        requestLayout();
    }

    @Override
    protected void onDraw(@NonNull Canvas c) {
        super.onDraw(c);

        if (insets != null && insets.getSystemWindowInsetTop() > 0) {
            statusBarBackground.setBounds(0, 0, getWidth(), insets.getSystemWindowInsetTop());
            statusBarBackground.draw(c);
        }
    }
}
