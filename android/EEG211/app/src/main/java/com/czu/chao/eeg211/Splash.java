package com.czu.chao.eeg211;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.media.MediaPlayer;
import android.os.CountDownTimer;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.animation.AnimationUtils;
import android.widget.ImageView;
import android.widget.TextView;

import com.daimajia.androidanimations.library.Techniques;
import com.daimajia.androidanimations.library.YoYo;
import com.nineoldandroids.animation.Animator;




/*
app for monitor the EEG&EMG
auther: Mi Chao <mc0514@163.com>
Date:   2016-5-9
 */



public class Splash extends AppCompatActivity {

    Intent intent;
    int check;
    TextView tv;
    ImageView iv;
    MediaPlayer hb;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_splash);
        tv=(TextView) findViewById(R.id.tv_welcome);
        hb=MediaPlayer.create(this,R.raw.heartbeat);
        hb.start();
        tv.startAnimation(AnimationUtils.loadAnimation(Splash.this, android.R.anim.fade_in));
        iv = (ImageView) findViewById(R.id.imageView);

        YoYo.with(Techniques.FadeIn).duration(1000).withListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationStart(Animator animator) {

            }

            @Override
            public void onAnimationEnd(Animator animator) {
                YoYo.with(Techniques.Pulse).duration(1000).withListener(new Animator.AnimatorListener() {
                    @Override
                    public void onAnimationStart(Animator animator) {

                    }

                    @Override
                    public void onAnimationEnd(Animator animator) {
                        YoYo.with(Techniques.Pulse).duration(1000).withListener(new Animator.AnimatorListener() {
                            @Override
                            public void onAnimationStart(Animator animator) {

                            }

                            @Override
                            public void onAnimationEnd(Animator animator) {
                                YoYo.with(Techniques.Pulse).duration(1000).withListener(new Animator.AnimatorListener() {
                                    @Override
                                    public void onAnimationStart(Animator animation) {

                                    }

                                    @Override
                                    public void onAnimationEnd(Animator animation) {

                                    }

                                    @Override
                                    public void onAnimationCancel(Animator animation) {

                                    }

                                    @Override
                                    public void onAnimationRepeat(Animator animation) {

                                    }
                                }).playOn(findViewById(R.id.imageView));
                            }

                            @Override
                            public void onAnimationCancel(Animator animator) {

                            }

                            @Override
                            public void onAnimationRepeat(Animator animator) {

                            }
                        }).playOn(findViewById(R.id.imageView));
                    }

                    @Override
                    public void onAnimationCancel(Animator animator) {

                    }

                    @Override
                    public void onAnimationRepeat(Animator animator) {

                    }
                }).playOn(findViewById(R.id.imageView));
            }

            @Override
            public void onAnimationCancel(Animator animator) {

            }

            @Override
            public void onAnimationRepeat(Animator animator) {

            }
        }).playOn(findViewById(R.id.imageView));

        SharedPreferences preferences = getSharedPreferences("EEG", Context.MODE_PRIVATE);

        SharedPreferences.Editor editor = preferences.edit();
        editor.putInt("Check", 0);// for debug the login activity

        editor.commit();
        check = preferences.getInt("Check", 0);

        new CountDownTimer(5500, 1000) {

            @Override
            public void onTick(long millisUntilFinished) {

            }

            //   @Override
            public void onFinish() {
//                if (check == 0) {
//                    intent = new Intent(Splash.this, Login.class);
//                    startActivity(intent);
//                    hb.stop();
//                    finish();
//                } else {
//                    intent = new Intent(Splash.this, ListActivity.class);
//                    startActivity(intesnt);
//                    hb.stop();
//                    finish();
//
//
//                }

                intent = new Intent(Splash.this,ListActivity.class);
                startActivity(intent);
                hb.stop();
                finish();
            }

        }.start();


    }
}
