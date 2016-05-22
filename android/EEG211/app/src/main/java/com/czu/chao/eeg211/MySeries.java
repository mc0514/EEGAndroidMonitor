package com.czu.chao.eeg211;

import android.util.Pair;

import com.androidplot.xy.SimpleXYSeries;

/**
 * Created by chao on 16-5-12.
 */
public class MySeries extends SimpleXYSeries{
     int maxSize=20;

    public MySeries(String title) {
        super(title);
    }

    public int getMaxSize()
    {
        return maxSize;
    }



    @Override
    public Pair<Number, Number> removeLast() {
        return super.removeLast();
    }
}
