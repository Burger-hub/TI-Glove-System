using UnityEngine;

public class GrabbableObject : MonoBehaviour
{
    public bool IsGrabbed;
    public Vector3 grabPositionOffset; // ���ֲ��������λ��ƫ��
    public Vector3 grabRotationOffset; // ���ֲ����������תƫ��

    // Ӧ��ץȡƫ��
    public void ApplyGrabPos(Transform hand)
    {
        transform.localPosition = grabPositionOffset;
        transform.localRotation = Quaternion.Euler(grabRotationOffset);
        
    }
}
