using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Meter : MonoBehaviour
{
    public Transform pointer;
    public Transform startPoint;
    public Transform endPoint;
    public float startAngle = 0, endAngle = 0;
    public float pointerValue = 0;
    public float startValue = 0, endValue = 1;
    [Range(0, 10)]
    public float power = 1;
    public bool reverse = false;
    public FingerController fingerController;
    Vector3 pointerAngle;
    Vector3 StartAngle, EndAngle;
    private void Start()
    {
        mPointerValue = pointerValue;
    }
    public void UpdateMeter()
    {
        LimitAngle(ref startAngle);
        if (endAngle > startAngle)
            endAngle = startAngle;
        if (endAngle < startAngle - 360)
            endAngle = startAngle - 360;
        if (pointerValue > endValue)
            pointerValue = endValue;
        if (pointerValue < startValue)
            pointerValue = startValue;

        StartAngle = Vector3.zero;
        EndAngle = Vector3.zero;
        StartAngle.z = startAngle;
        EndAngle.z = endAngle;

        float percentage = (pointerValue - startValue) / (endValue - startValue);//得到百分比
        fingerController.UpdatePoint(percentage);
        if (reverse)
            percentage = 1- percentage;
        pointerAngle.z = EndAngle.z + (Mathf.Abs(EndAngle.z) + StartAngle.z) * Mathf.Pow(percentage, power);//转换为角度

        LimitAngle(pointer, ref pointerAngle);
        LimitAngle(startPoint, ref StartAngle);
        LimitAngle(endPoint, ref EndAngle);
    }

    public void UpdateMeterLerp(float value)
    {
        mPointerValue = value;
        isLerp = true;
    }
    bool isLerp = false;
    float mPointerValue = 0;
    private void Update()
    {
        if (isLerp)
        {
            pointerValue = Mathf.Lerp(pointerValue, mPointerValue, 20f*Time.deltaTime);
            if (Mathf.Abs(pointerValue- mPointerValue) < 0.01f)
            {
                pointerValue = mPointerValue;
                UpdateMeter();
                isLerp = false;
            }
            UpdateMeter();
        }
    }
    void LimitAngle(ref float angle)
    {
        if (angle > 360)
        {
            angle -= 360;
        }
        if (angle < -360)
        {
            angle += 360;
        }
    }
    void LimitAngle(Transform target, ref Vector3 angle)
    {
        if (angle.z > 360)
        {
            angle.z -= 360;
        }
        if (angle.z < -360)
        {
            angle.z += 360;
        }
        target.localEulerAngles = angle;
    }
    private void OnValidate()
    {
        UpdateMeter();
    }
}
