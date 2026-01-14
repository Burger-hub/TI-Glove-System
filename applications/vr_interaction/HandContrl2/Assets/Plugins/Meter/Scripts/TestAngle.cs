using System.Collections;
using System.Collections.Generic;
using UnityEngine;
[ExecuteInEditMode]
public class TestAngle : MonoBehaviour
{
    public Meter meter;
    public Transform angle1, angle2, angle3, angle4, angle5;
    public bool isok = false;

    // Update is called once per frame
    void Update()
    {
        if (!isok)
            return;
        angle1.localEulerAngles = new Vector3(0, 0, meter.endAngle + (Mathf.Abs(meter.endAngle) + meter.startAngle) * Mathf.Pow(0.1f, meter.power));
        angle2.localEulerAngles = new Vector3(0, 0, meter.endAngle + (Mathf.Abs(meter.endAngle) + meter.startAngle) * Mathf.Pow(0.25f, meter.power));
        angle3.localEulerAngles = new Vector3(0, 0, meter.endAngle + (Mathf.Abs(meter.endAngle) + meter.startAngle) * Mathf.Pow(0.5f, meter.power));
        angle4.localEulerAngles = new Vector3(0, 0, meter.endAngle + (Mathf.Abs(meter.endAngle) + meter.startAngle) * Mathf.Pow(0.75f, meter.power));
        angle5.localEulerAngles = new Vector3(0, 0, meter.endAngle + (Mathf.Abs(meter.endAngle) + meter.startAngle) * Mathf.Pow(0.9f, meter.power));

    }
}
