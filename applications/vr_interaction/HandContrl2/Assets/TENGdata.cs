using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Net.Sockets;
using System.Net;

public class TENGdata : MonoBehaviour
{ 
    public GameObject[] Fingers;
    public DataReceiver dataReceiver;
    
    void Start()
    {
        dataReceiver = FindObjectOfType<DataReceiver>();
    }

    void Update()
    {
        SharedData sharedData = dataReceiver.sharedData;

        for (int i = 0; i < 5; i++)
        {
            Fingers[i].GetComponentInChildren<Meter>().UpdateMeterLerp(1-sharedData.fingerRotations[4-i]);
        }

        Debug.Log("Received: " + string.Join(", ", sharedData.fingerRotations));
        /*
        IPEndPoint ipEndPoint = new IPEndPoint(IPAddress.Any, port);
        byte[] data = udpClient.Receive(ref ipEndPoint);
        string message = System.Text.Encoding.ASCII.GetString(data);
        
        SharedData sharedData = dataReceiver.sharedData;
        // 解析接收到的浮点数
        string[] values = message.Split(' ');
        // 获取新的旋转角
        float newPer0 = 0.0f;
        float newPer1 = 0.0f;
        float newPer2 = 0.0f;
        float newPer3 = 0.0f;
        float newPer4 = 0.0f;
        newPer0 = float.Parse(values[0]);
        newPer1 = float.Parse(values[1]);
        newPer2 = float.Parse(values[2]);
        newPer3 = float.Parse(values[3]);
        newPer4 = float.Parse(values[4]);
        //percentage = newPercentage;
        Fingers[0].GetComponentInChildren<Meter>().UpdateMeterLerp(newPer0);
        Fingers[1].GetComponentInChildren<Meter>().UpdateMeterLerp(newPer1);
        Fingers[2].GetComponentInChildren<Meter>().UpdateMeterLerp(newPer2);
        Fingers[3].GetComponentInChildren<Meter>().UpdateMeterLerp(newPer3);
        Fingers[4].GetComponentInChildren<Meter>().UpdateMeterLerp(newPer4);
        
        // 在控制台输出接收到的浮点数
        Debug.Log("Received: " + newPer0+ newPer1+ newPer2+newPer3+ newPer4);
        */
    }
}
