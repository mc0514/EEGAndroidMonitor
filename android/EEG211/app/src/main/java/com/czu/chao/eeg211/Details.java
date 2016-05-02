package com.czu.chao.eeg211;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.net.Uri;
import android.os.Environment;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.Toast;

import com.androidplot.xy.BoundaryMode;
import com.androidplot.xy.LineAndPointFormatter;
import com.androidplot.xy.PointLabelFormatter;
import com.androidplot.xy.SimpleXYSeries;
import com.androidplot.xy.XYPlot;
import com.androidplot.xy.XYStepMode;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.FieldPosition;
import java.text.Format;
import java.text.ParsePosition;
import java.util.Arrays;
import java.util.Set;
import java.util.UUID;

public class Details extends AppCompatActivity implements View.OnClickListener {

    ImageButton b1, b2, b3;
    SharedPreferences sharedPreferences;
    String docmail, docnum, objname,objAge;
    Button a1, a2, a3, a4;


    BluetoothAdapter bluetoothAdapter;
    BluetoothDevice bluetoothDevice;
    BluetoothSocket bluetoothSocket;
    Set<BluetoothDevice> pairedDevice;
    //String address = "94:00:70:06:E3:74";
    String address = "98:D3:31:30:3F:87";
    android.os.Handler bluetoothIn;
    int HandlerState = 0;
    UUID BTMODULEUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    ConnectedThread connectedThread;


    XYPlot plot;
    SimpleXYSeries something;
    int max_size = 20;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_details);

        plot = (XYPlot) findViewById(R.id.ecgPlot);

        something = new SimpleXYSeries("EEG");
        //something.useImplicitXVals();
        plot.setRangeBoundaries(0, 100, BoundaryMode.FIXED);
        //plot.setDomainBoundaries(0, 20, BoundaryMode.FIXED);
        LineAndPointFormatter series1Format = new LineAndPointFormatter();
        series1Format.setPointLabelFormatter(new PointLabelFormatter());
        series1Format.configure(getApplicationContext(),
                R.xml.line_point_formatter_with_labels);

        //plot.addSeries(something, new LineAndPointFormatter(Color.rgb(200, 100, 100), null, null, null));
        plot.addSeries(something,series1Format);
        // plot.setDomainStepValue(5);
        //plot.setDomainStep(XYStepMode.INCREMENT_BY_VAL, 5);
        plot.setTicksPerRangeLabel(3);


        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

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


        a1.setText("Enable bluetooth");
        a1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!bluetoothAdapter.isEnabled()) {
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
                pairedDevice = bluetoothAdapter.getBondedDevices();
                Log.v("bluetooth", "size" + pairedDevice.size());
                for (BluetoothDevice device : pairedDevice) {
                    Log.v("bluetooth", "name: " + device.getName());
                    if (device.getName().equals("EMG-slave")) {
                        //address = device.getAddress();
                        Log.v("bluetooth", "address: " + device.getAddress());
                        Toast.makeText(Details.this, address, Toast.LENGTH_SHORT).show();
                        bluetoothDevice = bluetoothAdapter.getRemoteDevice(address);
                        Log.v("bluetooth", "Remote++++bluetoothDevice: " + bluetoothDevice.getName());
                        try {
                            bluetoothSocket = createBluetoothSocket(bluetoothDevice);
                            Log.v("bluetooth", "bluetoothsocket created ");

                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                        try {
                            bluetoothSocket.connect();
                            Log.v("bluetooth", "bluetoothsocket connected");
                        } catch (IOException e) {
                            e.printStackTrace();
                        }

                        connectedThread = new ConnectedThread(bluetoothSocket);
                        connectedThread.start();
                        //Toast.makeText(Details.this, "Connect success!", Toast.LENGTH_SHORT).show();

                    }
                    //connectedThread = new ConnectedThread(bluetoothSocket);
                    //connectedThread.start();
                }
                //connectedThread = new ConnectedThread(bluetoothSocket);
                //connectedThread.start();


            }
        });


        a3.setText("Start");
        a3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                connectedThread.write("A");

            }
        });

        a4.setText("Stop");
        a4.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                connectedThread.write("S");

            }
        });

        bluetoothIn = new Handler() {

            public void HandleMessage(Message msg) {
                if (msg.what == HandlerState) {
                }
            }

        };


    }

    private BluetoothSocket createBluetoothSocket(BluetoothDevice device) throws IOException {

        return device.createRfcommSocketToServiceRecord(BTMODULEUUID);
        //creates secure outgoing connecetion with BT device using UUID
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


    public class ConnectedThread extends Thread {
        InputStream inputStream;
        OutputStream outputStream;
        FileWriter writer;
        final byte delimiter = 10;
        byte[] readBuffer;
        int readBufferPosition;

        public ConnectedThread(BluetoothSocket bluetoothSocket1) {
            try {
                inputStream = bluetoothSocket1.getInputStream();
                outputStream = bluetoothSocket1.getOutputStream();
            } catch (IOException e) {
                e.printStackTrace();
            }
            try {
                File root = new File(Environment.getExternalStorageDirectory(), "EEG");
                if (!root.exists()) {
                    root.mkdirs();
                }
                File gpxfile = new File(root, "eeg.txt");
                writer = new FileWriter(gpxfile);
                writer.append("Report of " + objname + "Age: "+objAge+"\n");
//                writer.flush();
//                writer.close();
//                Toast.makeText(MainActivity4.this, "Saved", Toast.LENGTH_SHORT).show();
            } catch (IOException e) {
                e.printStackTrace();
            }

        }

        public void run() {

            int counter = 0;
            readBuffer = new byte[1024];
            readBufferPosition = 0;
            //byte[] buffer = new byte[1024];
            int bytes;
            MyIndexFormat mif = new MyIndexFormat ();
            while (true) {
                try {

                    // bytes = inputStream.read(buffer);
                    bytes = inputStream.available();
                    //Log.d("tag","bytes: " + bytes);

                    //String readMessage = new String(buffer, 0, bytes);
                    if (bytes > 0) {
                        byte[] buffer = new byte[bytes];
                        inputStream.read(buffer);
                        for (int i = 0; i < bytes; i++) {
                            byte b = buffer[i];
                            if (b == delimiter) {
                                byte[] encodedBytes = new byte[readBufferPosition - 1];
                                System.arraycopy(readBuffer, 0, encodedBytes, 0, encodedBytes.length);
                                final String data = new String(encodedBytes, "US-ASCII");
                                readBufferPosition = 0;
                                int xyz = Integer.parseInt(data);
                                counter += 5;
                                Log.v("tag", "counter: " + counter);
                                writer.append(xyz + " ");
                                if (something.size() > max_size) {
                                    something.removeFirst();
                                }
                                //something.addLast(null, xyz);
                                something.addLast(counter,xyz);
                                //Log.d("tag", "readMessage" +Integer.parseInt(readMessage));
                                //omething.addLast(null, Integer.parseInt(readMessage));
                                //String[] mylable=new String [10];
                                //mylable[0]="10";
                                //mif.Labels =mylable;
                                //plot.getGraphWidget().setDomainValueFormat(mif);

                                plot.redraw();
                                bluetoothIn.obtainMessage(HandlerState, bytes, -1, data).sendToTarget();
                                write("*");

                            } else {

                                readBuffer[readBufferPosition++] = b;
                            }
                        }
                    }
                    //readMessage="0 0.1 0.2 0.3 0.2 0.1";
                    //bytes=readMessage.length();
                    //String temp="";
                    //for (int i=0;i<bytes;i++)
                    //{
                    //temp+=buffer[i];
                    //}
                    //Log.d("tag","data: "+data);
                    //int xyz = buffer[0];
                    //int xyz=Integer.parseInt(readMessage);
                    // Log.d("tag", readMessage + " xyz" + xyz);
                    //writer.append(xyz+" ");
//                    writer.flush();
//                    writer.close();


                    //if (something.size() > max_size) {
                    //something.removeFirst();
                    //}
                    // something.addLast(null, xyz);
                    //Log.d("tag", "readMessage" +Integer.parseInt(readMessage));
                    //omething.addLast(null, Integer.parseInt(readMessage));
                    // plot.redraw();
                    // bluetoothIn.obtainMessage(HandlerState, bytes, -1, readMessage).sendToTarget();


                } catch (IOException e) {
                    e.printStackTrace();
                }
            }

        }

        public void write(String input) {
            byte[] msg = input.getBytes();
            try {
                outputStream.write(msg);
                //outputStream.write();
                if (input.equals("S")) {
                    writer.flush();
                    writer.close();

                }

            } catch (IOException e) {
                Toast.makeText(Details.this, "Failed to send", Toast.LENGTH_SHORT).show();
                e.printStackTrace();
            }
        }

    }

    public class MyIndexFormat extends Format {

        public String[] Labels = null;

        @Override
        public StringBuffer format(Object obj,
                                   StringBuffer toAppendTo,
                                   FieldPosition pos) {

            // try turning value to index because it comes from indexes
            // but if is too far from index, ignore it - it is a tick between indexes
            float fl = ((Number) obj).floatValue();
            int index = Math.round(fl);
            if (Labels == null || Labels.length <= index ||
                    Math.abs(fl - index) > 0.1)
                return new StringBuffer("");

            return new StringBuffer(Labels[index]);
        }

        @Override
        public Object parseObject(String string, ParsePosition position) {
            return null;
        }
    }
}
