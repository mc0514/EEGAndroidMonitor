package com.czu.chao.eeg211;

import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.Toast;
import com.androidplot.xy.BoundaryMode;
import com.androidplot.xy.LineAndPointFormatter;
import com.androidplot.xy.PointLabelFormatter;
import com.androidplot.xy.XYPlot;
import android.util.Log;

public class Details extends AppCompatActivity implements View.OnClickListener {

    ImageButton b1, b2, b3;
    SharedPreferences sharedPreferences;
    String docmail, docnum, objname,objAge;
    Button a1, a2, a3, a4;
    //BluetoothAdapter bluetoothAdapter;
    BluetoothConnect bluetoothConnect;
    android.os.Handler bluetoothIn;
    ConnectThread connectedThread;
    XYPlot plot;
    MySeries something;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_details);
        plot = (XYPlot) findViewById(R.id.ecgPlot);
        something = new MySeries("EEG");
        plot.setRangeBoundaries(0, 100, BoundaryMode.FIXED);;
        LineAndPointFormatter series1Format = new LineAndPointFormatter();
        series1Format.setPointLabelFormatter(new PointLabelFormatter());
        series1Format.configure(getApplicationContext(),
                R.xml.line_point_formatter_with_labels);
        plot.addSeries(something,series1Format);
        plot.setTicksPerRangeLabel(3);
        a1 = (Button) findViewById(R.id.button);
        a2 = (Button) findViewById(R.id.button2);
        a3 = (Button) findViewById(R.id.button3);
        a4 = (Button) findViewById(R.id.button4);
        b1 = (ImageButton) findViewById(R.id.imageButton);
        b2 = (ImageButton) findViewById(R.id.imageButton2);
        b3 = (ImageButton) findViewById(R.id.map);
        b1.setOnClickListener(this);
        b2.setOnClickListener(this);
        b3.setOnClickListener(this);
        sharedPreferences = getSharedPreferences("EEG", Context.MODE_PRIVATE);
        objname = sharedPreferences.getString("objname", "Invalid");
        objAge=sharedPreferences.getString("objAge", "Invalid");
        docmail = sharedPreferences.getString("docmail", "Invalid");
        docnum = sharedPreferences.getString("docnumber", "Invalid");
        //bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        bluetoothConnect=new BluetoothConnect();
        a1.setText("Enable bluetooth");
        a1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!bluetoothConnect.isEnabled()) {
                    Intent on = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                    startActivityForResult(on, 5);
                } else {
                    Toast.makeText(Details.this, "Already on", Toast.LENGTH_SHORT).show();
                }

            }
        });
        a2.setText("Pair");
        a2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(!bluetoothConnect.isEnabled()){
                bluetoothConnect.pair();
                connectedThread = new ConnectThread(bluetoothConnect.getBluetoothSocket());
                connectedThread.start(something);
                    }else{
                    Toast.makeText(Details.this,"please enable bluetooth!",Toast.LENGTH_SHORT).show();
                }
            }
                    //connectedThread = new ConnectedThread(bluetoothSocket);
                    //connectedThread.start();

                //connectedThread = new ConnectedThread(bluetoothSocket);
                //connectedThread.start();



        });


        a3.setText("Start");
        a3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(!bluetoothConnect.isConnected()){
                connectedThread.write("A");
            }else{
                    Toast.makeText(Details.this,"please connect the bluetooth",Toast.LENGTH_SHORT).show();
                }
            }
        });

        a4.setText("Stop");
        a4.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(!bluetoothConnect.isConnected()){
                connectedThread.write("S");

            }else{
                    Toast.makeText(Details.this,"please connect the bluetooth",Toast.LENGTH_SHORT).show();
                }
            }
        });



    }



    @Override
    public void onClick(View v) {

        switch (v.getId()) {
            case R.id.imageButton:
                Intent in = new Intent(Intent.ACTION_DIAL, Uri.parse("tel:" + docnum));
                startActivity(in);
                break;

            case R.id.imageButton2:
                String abc = "file://" + Environment.getExternalStorageDirectory() + "/EEG/eeg.txt";
                Intent intent = new Intent(Intent.ACTION_SEND);
                intent.putExtra(Intent.EXTRA_EMAIL, new String[]{"mc0514@163,com"});
                intent.putExtra(Intent.EXTRA_SUBJECT, "EEG Report");
                intent.putExtra(Intent.EXTRA_TEXT, objname);
                intent.putExtra(Intent.EXTRA_STREAM, Uri.parse(abc));
                intent.setType("*/*");
                startActivity(intent);
                break;

            case R.id.map:
                String search = "geo:0,0?q=nearby hospital";
                Intent intent1 = new Intent(Intent.ACTION_VIEW, Uri.parse(search));
                startActivity(intent1);

                break;

        }

    }


}
