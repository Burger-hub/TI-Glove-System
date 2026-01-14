using System.Collections;
using System.Collections.Generic;
using UnityEngine;
//using System.Net.Sockets;
//using System.Net;
//using System.Threading;


public class imuData : MonoBehaviour
{
    /*
    private UdpClient udpClient;
    private int port = 5000;
    // Start is called before the first frame update
    public float xRotation = 0.0f;
    public float yRotation = 0.0f;
    public float zRotation = 0.0f;
    */
    public DataReceiver dataReceiver;
    void Start()
    {
        /*
        udpClient = new UdpClient(port);
        Thread thread = new Thread(IMUreceiver);
        thread.Start();
        */
        dataReceiver = FindObjectOfType<DataReceiver>();

    }
    /*
    private void IMUreceiver()
    {
        IPEndPoint ipEndPoint = new IPEndPoint(IPAddress.Any, port);
        Debug.Log("正在连接...");
        while (true) { 
            byte[] data = udpClient.Receive(ref ipEndPoint);
            string message = System.Text.Encoding.ASCII.GetString(data);
            // 解析接收到的浮点数
            string[] values = message.Split(' ');
            // 获取新的旋转角
            float newxRotation = 0.0f;
            float newyRotation = 0.0f;
            float newzRotation = 0.0f;
            newxRotation = float.Parse(values[6]);
            newyRotation = float.Parse(values[7]);
            newzRotation = float.Parse(values[8]);
            xRotation = newxRotation;
            zRotation = -newyRotation;
            yRotation = newzRotation;//转换为左手系
        }
    }
    */
    // Update is called once per frame
    void Update()
    {
        /*
        //transform.rotation = Quaternion.Euler(xRotation, yRotation, zRotation);
        Vector3 imuRatate=new Vector3(xRotation, yRotation, zRotation);
        transform.localEulerAngles = imuRatate;//改变物体欧拉角，相对于父级坐标系
        Vector3 euler = transform.eulerAngles;
        //Debug.Log(euler);
        // 在控制台输出接收到的浮点数
        Debug.Log("Received: " + xRotation + ", " + yRotation + ", " + zRotation);
        */
        SharedData sharedData = dataReceiver.sharedData;
        transform.localEulerAngles = new Vector3(sharedData.xRotation, sharedData.yRotation, sharedData.zRotation);
        Debug.Log("Received: " + sharedData.xRotation + ", " + sharedData.yRotation + ", " + sharedData.zRotation);
    }
}
