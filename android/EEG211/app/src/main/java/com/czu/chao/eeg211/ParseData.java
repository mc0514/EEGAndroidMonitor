package com.czu.chao.eeg211;

import android.util.Log;

import java.io.IOException;

/**
 * Created by chao on 16-5-9.
 */
public class ParseData {

    int bytes;
    byte[] buffer;
    byte delimiter;
    int readBufferPosition=0;
    byte[] readBuffer=new byte[1024];
    int resultData;

    public ParseData(int byteslong, byte[] readbuffer)
    {
        bytes=byteslong;
        buffer=readbuffer;
        if (bytes > 0) {
            byte[] buffer = new byte[bytes];
            //inputStream.read(buffer);
            for (int i = 0; i < bytes; i++) {
                byte b = buffer[i];
                if (b == delimiter) {
                    byte[] encodedBytes = new byte[readBufferPosition - 1];
                    System.arraycopy(readBuffer, 0, encodedBytes, 0, encodedBytes.length);
                    try {
                        final String data = new String(encodedBytes, "US-ASCII");
                        readBufferPosition = 0;
                        resultData = Integer.parseInt(data);
                    }catch (IOException e){
                        e.printStackTrace();
                    }



                } else {

                    readBuffer[readBufferPosition++] = b;
                }
            }
        }
    }

    public int getResultData()
    {
        return resultData;
    }

}
