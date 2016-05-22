package com.czu.chao.eeg211;

import android.bluetooth.BluetoothSocket;
import android.os.Environment;
import android.util.Log;
import android.widget.Toast;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Created by chao on 16-5-9.
 */
public class ConnectThread extends Thread{
    InputStream inputStream;
    OutputStream outputStream;

    ParseData parseData;
    int bytes;
    byte[] buffer;
    public ConnectThread(BluetoothSocket bluetoothSocket) {
        try {
            inputStream = bluetoothSocket.getInputStream();
            outputStream = bluetoothSocket.getOutputStream();
        } catch (IOException e) {
            e.printStackTrace();
        }
        /*try {
            File root = new File(Environment.getExternalStorageDirectory(), "EEG");
            if (!root.exists()) {
                root.mkdirs();
            }
            //File gpxfile = new File(root, "eeg.txt");
            //writer = new FileWriter(gpxfile);
            //writer.append("Report of " + objname + "Age: "+objAge+"\n");
//                writer.flush();
//                writer.close();
//                Toast.makeText(MainActivity4.this, "Saved", Toast.LENGTH_SHORT).show();
        } catch (IOException e) {
            e.printStackTrace();
        }*/

    }


   public void read()
   {
       try {
           bytes = inputStream.available();

           if (bytes > 0) {
               buffer = new byte[bytes];
               inputStream.read(buffer);
           }
       }catch (IOException e)
       {
           e.printStackTrace();
       }
   }



    public void write(String input) {
        byte[] msg = input.getBytes();
        try {
            outputStream.write(msg);
            //outputStream.write();


        } catch (IOException e) {
            //Toast.makeText(Details.this, "Failed to send", Toast.LENGTH_SHORT).show();
            e.printStackTrace();
        }
    }

    public void start(MySeries myseries)
    {

        int counter=0;
        while(true)
        {


                    read();
                    if (myseries.size() > myseries.getMaxSize())
                    {
                        myseries.removeFirst();
                    }

                    myseries.addLast(counter,parseData.getResultData());
                    counter=counter+5;



        }

    }

}

