package com.czu.chao.eeg211;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;

import com.daimajia.androidanimations.library.Techniques;
import com.daimajia.androidanimations.library.YoYo;
import com.nineoldandroids.animation.Animator;

public class Login extends AppCompatActivity {

    ImageButton button;
    EditText id,e1,e2,e3,e5;
    TextView t1,t2,t3,t4;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);

        button= (ImageButton) findViewById(R.id.bt);
        id= (EditText) findViewById(R.id.et2);
        e5= (EditText) findViewById(R.id.et5);
        e1= (EditText) findViewById(R.id.et);
        e2= (EditText) findViewById(R.id.et3);
        e3= (EditText) findViewById(R.id.et4);
        t1= (TextView) findViewById(R.id.textView2);
        t2= (TextView) findViewById(R.id.textView3);
        t3= (TextView) findViewById(R.id.textView4);
        t4= (TextView) findViewById(R.id.textView5);

        YoYo.with(Techniques.Pulse).duration(1000).withListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationStart(Animator animator) {

            }

            @Override
            public void onAnimationEnd(Animator animator) {

            }

            @Override
            public void onAnimationCancel(Animator animator) {

            }

            @Override
            public void onAnimationRepeat(Animator animator) {

            }
        }).playOn(findViewById(R.id.bt));


        button.setOnClickListener(new View.OnClickListener() {
            @Override


            public void onClick(View v) {
                String mynumber = e1.getText().toString();
                String docnumber = e3.getText().toString();
                String docmail = e5.getText().toString();

//                if (mynumber.equals("") || mynumber == null) {
//                    Toast.makeText(Login.this, "Enter Your Phone Number", Toast.LENGTH_SHORT).show();
//                } else if (mynumber.length() < 10 || mynumber.length() > 10) //id.matches(reg)==false) {
//                {
//                    Toast.makeText(Login.this, "Enter valid phone number", Toast.LENGTH_SHORT).show();
//                } else {
                SharedPreferences preferences = getSharedPreferences("Yes", Context.MODE_PRIVATE);
                SharedPreferences.Editor editor = preferences.edit();
                editor.putInt("Check", 1);
                editor.putString("myname", mynumber);
                editor.putString("docnumber", docnumber);
                editor.putString("docmail", docmail);
                editor.commit();

                Intent intent = new Intent(Login.this, Details.class);
                startActivity(intent);
                finish();
            }

        });
    }
}
