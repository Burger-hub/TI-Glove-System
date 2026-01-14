using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[ExecuteInEditMode]
public class ShowMeterValue : MonoBehaviour
{
    public TextMesh startText, endText, pointerText;
    public Meter meter;
    void Update()
    {
        if (endText && startText && pointerText)
        {
            endText.transform.position = meter.endPoint.GetChild(0).transform.position;
            startText.transform.position = meter.startPoint.GetChild(0).transform.position;
            endText.text = meter.endValue.ToString();
            startText.text = meter.startValue.ToString();
            pointerText.text = meter.pointerValue.ToString();
        }
        if (meter)
            meter.UpdateMeter();
    }
}
