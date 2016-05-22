package com.czu.chao.eeg211;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;

public class ListActivity extends AppCompatActivity {

    ImageButton BT_monitor;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_list);

        BT_monitor=(ImageButton)findViewById(R.id.BT_Monitor);

        BT_monitor.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v) {
                Intent intent=new Intent(ListActivity.this, Details.class);
                startActivity(intent);
            }


        });
    }
}
