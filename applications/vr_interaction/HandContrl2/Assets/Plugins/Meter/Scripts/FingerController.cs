using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FingerController : MonoBehaviour
{
    public Transform FingerClonePoint;
    public Transform FingerCloneRotate;
    private Quaternion FingerRotation = Quaternion.identity;
    
    public void UpdatePoint(float percentage)
    {
        if (FingerRotation== Quaternion.identity)
            FingerRotation = transform.rotation;
        transform.position = FingerClonePoint.position;
        transform.rotation = Quaternion.Lerp(FingerRotation, FingerCloneRotate.rotation, percentage);
    }
}
