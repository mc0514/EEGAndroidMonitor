package com.czu.chao.eeg211;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.util.Log;
import android.widget.Toast;

import java.io.IOException;
import java.util.Set;
import java.util.UUID;

/**
 * Created by chao on 16-5-9.
 */
public class BluetoothConnect {


    BluetoothAdapter bluetoothAdapter;
    BluetoothDevice bluetoothDevice;
    BluetoothSocket bluetoothSocket;
    Set<BluetoothDevice> pairedDevice;
    String address = "98:D3:31:30:3F:87";
    android.os.Handler bluetoothIn;
    int HandlerState = 0;
    UUID BTMODULEUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    public void pair()
    {
        pairedDevice = bluetoothAdapter.getBondedDevices();
        Log.v("bluetooth", "size" + pairedDevice.size());
        for (BluetoothDevice device : pairedDevice) {
            Log.v("bluetooth", "name: " + device.getName());
            if (device.getName().equals("EMG-slave")) {
                //address = device.getAddress();
                Log.v("bluetooth", "address: " + device.getAddress());
                //Toast.makeText(BluetoothConnect.this, address, Toast.LENGTH_SHORT).show();
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

                //connectedThread = new ConnectedThread(bluetoothSocket);
                //connectedThread.start();
                //Toast.makeText(Details.this, "Connect success!", Toast.LENGTH_SHORT).show();

            }
            //connectedThread = new ConnectedThread(bluetoothSocket);
            //connectedThread.start();
        }

    }

    public boolean isEnabled()
    {
        return bluetoothAdapter.isEnabled();
    }

    public BluetoothSocket getBluetoothSocket()
    {
        return bluetoothSocket;
    }

    private BluetoothSocket createBluetoothSocket(BluetoothDevice device) throws IOException {

        return device.createRfcommSocketToServiceRecord(BTMODULEUUID);
        //creates secure outgoing connecetion with BT device using UUID
    }
}
