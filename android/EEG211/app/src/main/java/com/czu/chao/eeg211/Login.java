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
import android.widget.Toast;

import com.daimajia.androidanimations.library.Techniques;
import com.daimajia.androidanimations.library.YoYo;
import com.nineoldandroids.animation.Animator;

public class Login extends AppCompatActivity {

    ImageButton buttonGo;
    EditText et_objname,et_objAge,et_docTel,et_docEmatil,et_objTel;
    //TextView t1,t2,t3,t4;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);

        buttonGo= (ImageButton) findViewById(R.id.bt_loginToDetail);
        et_objTel= (EditText) findViewById(R.id.et_objectTel);

        et_objname= (EditText) findViewById(R.id.et_objectname);
        et_objAge=(EditText)findViewById(R.id.et_objectAge);
        et_docEmatil= (EditText) findViewById(R.id.et_doctorEmail);
        //e2= (EditText) findViewById(R.id.et_doctorTel);
        et_docTel= (EditText) findViewById(R.id.et_doctorTel);
        //t1= (TextView) findViewById(R.id.tv_objectname);
        //t2= (TextView) findViewById(R.id.tv_objectTel);
       // t3= (TextView) findViewById(R.id.tv_objectAge);
        //t4= (TextView) findViewById(R.id.tv_doctorTel);

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
        }).playOn(findViewById(R.id.bt_loginToDetail));


        buttonGo.setOnClickListener(new View.OnClickListener() {
            @Override


            public void onClick(View v) {
                String objname = et_objname.getText().toString();
                String objTel=et_objTel.getText().toString();
                String docTel = et_docTel.getText().toString();
                String docmail = et_docEmatil.getText().toString();
                String objAge=et_objAge.getText().toString();

                if (objTel.equals("") || objTel == null) {
                    Toast.makeText(Login.this, "Enter Your Phone Number", Toast.LENGTH_SHORT).show();
               } else if (objTel.length() < 8 ) //id.matches(reg)==false) {
                {
                    Toast.makeText(Login.this, "Enter valid phone number", Toast.LENGTH_SHORT).show();
                } else {
                SharedPreferences preferences = getSharedPreferences("EEG", Context.MODE_PRIVATE);
                SharedPreferences.Editor editor = preferences.edit();
                editor.putInt("Check", 1);
                editor.putString("objname", objname);
                    editor.putString("objAge",objAge);
                editor.putString("docnumber", docTel);
                editor.putString("docmail", docmail);

                editor.commit();

                Intent intent = new Intent(Login.this, Details.class);
                startActivity(intent);
                finish();
            }

        }});
    }
}

