using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class UI_FingerChangeAngle : MonoBehaviour
{
    InputField InputField;
    public GameObject[] Fingers;
    void Start()
    {
        foreach (var item in GetComponentsInChildren<InputField>())
        {
            if (item.name== "InputFieldAngle")
            {
                InputField = item;
                InputField.onValueChanged.AddListener(InputFieldValue);
            }
        }
        foreach (var item in GetComponentsInChildren<Slider>())
        {
            if (item.name == "SliderPower")
            {
                item.onValueChanged.AddListener(SliderPowerValue);
            }
        }
        foreach (var item in GetComponentsInChildren<Button>())
        {
            switch (item.name)
            {
                case "Button1":
                    item.onClick.AddListener(Button1);
                    break;
                case "Button2":
                    item.onClick.AddListener(Button2);
                    break;
                case "Button3":
                    item.onClick.AddListener(Button3);
                    break;
                case "Button4":
                    item.onClick.AddListener(Button4);
                    break;
                case "Button5":
                    item.onClick.AddListener(Button5);
                    break;
                case "Button6":
                    item.onClick.AddListener(Button6);
                    break;
                case "Button7":
                    item.onClick.AddListener(Button7);
                    break;
                case "Button8":
                    item.onClick.AddListener(Button8);
                    break;
                case "Button9":
                    item.onClick.AddListener(Button9);
                    break;
                case "Button10":
                    item.onClick.AddListener(Button10);
                    break;
                case "展开":
                    item.onClick.AddListener(Stretch);
                    break;
                case "握拳":
                    item.onClick.AddListener(Grip);
                    break;
            }
        }
    }

    private void SliderPowerValue(float arg0)
    {
        Fingers[FingerIndex - 1].GetComponentInChildren<Meter>().UpdateMeterLerp(arg0);
        this.angle = arg0;
        SetFingerAngle();
    }

    int FingerIndex = 1;
    float angle = 1;
    private void InputFieldValue(string arg0)
    {
        int angle = int.Parse(arg0);
        Fingers[FingerIndex-1].GetComponentInChildren<Meter>().UpdateMeterLerp(angle);
        this.angle = angle;
    }

    void SetFingerAngle()
    {
        Fingers[FingerIndex - 1].GetComponentInChildren<Meter>().UpdateMeterLerp(angle);
    }

    private void Grip()
    {
        foreach (var item in Fingers)
        {
            item.GetComponentInChildren<Meter>().UpdateMeterLerp(item.GetComponentInChildren<Meter>().startValue);
        }
    }

    private void Stretch()
    {
        foreach (var item in Fingers)
        {
            item.GetComponentInChildren<Meter>().UpdateMeterLerp(item.GetComponentInChildren<Meter>().endValue);
        }
    }

    private void Button10()
    {
        FingerIndex = 10;
        SetFingerAngle();
    }

    private void Button9()
    {
        FingerIndex = 9; SetFingerAngle();
    }

    private void Button8()
    {
        FingerIndex = 8; SetFingerAngle();
    }

    private void Button7()
    {
        FingerIndex = 7; SetFingerAngle();
    }

    private void Button6()
    {
        FingerIndex = 6; SetFingerAngle();
    }

    private void Button5()
    {
        FingerIndex = 5; SetFingerAngle();
    }

    private void Button4()
    {
        FingerIndex = 4; SetFingerAngle();
    }

    private void Button3()
    {
        FingerIndex = 3; SetFingerAngle();
    }

    private void Button2()
    {
        FingerIndex = 2; SetFingerAngle();
    }

    private void Button1()
    {
        FingerIndex = 1; SetFingerAngle();
    }
}
