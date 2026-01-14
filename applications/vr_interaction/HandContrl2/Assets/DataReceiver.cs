using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Net.Sockets;
using System.Net;
using System.Threading;

public class DataReceiver : MonoBehaviour
{
    public bool IsSimulator;
    private UdpClient udpClient;
    private int port = 5006;
    public SharedData sharedData;
    // 布尔变量用于跟踪是否已经初始化过UdpClient对象
    private bool udpClientInitialized = false;

    void Start()
    {
        sharedData = new SharedData();
        if (IsSimulator)
        {
            // 使用模拟器
            Debug.Log("Using mouse simulator.");
        }
        else
        {
            // 确保只初始化一次UdpClient对象
            if (!udpClientInitialized)
            {
                udpClient = new UdpClient(port);
                udpClientInitialized = true;
            }



            // 在 Start() 方法中启动一个新的线程来接收数据
            Thread receiveThread = new Thread(ReceiveData);
            receiveThread.IsBackground = true;
            receiveThread.Start();
        }
    
    }
    void Update()
    {
        if (IsSimulator)
        {
            // 更新数据来自鼠标
            HandleMouseInput();
        }
    }

    private void HandleMouseInput()
    {
        // 模拟数据更新
        sharedData.yRotation += Input.GetAxis("Mouse X") * 5.0f;
        sharedData.zRotation -= Input.GetAxis("Mouse Y") * 5.0f;
        float targetFingerValue = Input.GetMouseButton(0) ? 1.0f : (Input.GetMouseButton(1) ? 0.0f : sharedData.fingerRotations[0]);

        for (int i = 0; i < 5; i++)
        {
            sharedData.fingerRotations[i] = Mathf.MoveTowards(sharedData.fingerRotations[i], targetFingerValue, 5.0f * Time.deltaTime);
        }

        if (Input.GetMouseButton(2))
        {
            for (int i = 0; i < 5; i++)
            {
                sharedData.fingerRotations[i] = 0f;
            }
        }
    }
    // 接收数据的方法
    private void ReceiveData()
    {
        IPEndPoint ipEndPoint = new IPEndPoint(IPAddress.Any, port);
        Debug.Log("Connecting...");
        while (true)
        {
            byte[] data = udpClient.Receive(ref ipEndPoint);
            string message = System.Text.Encoding.ASCII.GetString(data);
            Debug.Log("Received: " + message);
            string[] values = message.Split(' ');
            // 将接收到的数据存储在共享数据对象中
            lock (sharedData)
            {
                sharedData.xRotation = float.Parse(values[7]);
                sharedData.yRotation = -float.Parse(values[5]);
                sharedData.zRotation = float.Parse(values[6]);

                for (int i = 0; i < 5; i++)
                {
                    sharedData.fingerRotations[i] = float.Parse(values[i]);
                }
            }
        }
    }
}

public class SharedData
{
    public float xRotation { get; set; }
    public float yRotation { get; set; }
    public float zRotation { get; set; }
    public float[] fingerRotations { get; set; }

    public SharedData()
    {
        fingerRotations = new float[5];
    }
}
