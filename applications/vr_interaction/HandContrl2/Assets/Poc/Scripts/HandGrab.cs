using UnityEngine;

public class HandGrab : MonoBehaviour
{
    [SerializeField] private Transform handParent;
    private GameObject grabbedObject = null;
    public DataReceiver dataReceiver; // �ⲿ����ϵͳ�ṩ�Ĺ�������
    [SerializeField] private float grabThreshold = 0.8f;
    [SerializeField] private float releaseThreshold = 0.5f;
    private Vector3 lastPosition;
    private Vector3 velocity;


    private void Start()
    {
        //�ͷ���ֵ������ץȡ��ֵ
        releaseThreshold = releaseThreshold > grabThreshold ? grabThreshold : releaseThreshold;
        lastPosition = transform.position;
    }
    private void Update()
    {
        // �����ֲ����ٶ�
        velocity = (transform.position - lastPosition) / Time.deltaTime;
        lastPosition = transform.position;

        if (grabbedObject != null && ShouldRelease())
        {
            ReleaseObject();
        }
        
    }

    private void OnTriggerEnter(Collider other)
    {
        if (other.gameObject.CompareTag("Grabbable") && grabbedObject == null && ShouldGrab())
        {
            GrabObject(other.gameObject);
        }
    }

    private void OnTriggerStay(Collider other)
    {
        if (other.gameObject.CompareTag("Grabbable") && grabbedObject == null && ShouldGrab())
        {
            GrabObject(other.gameObject);
        }
    }

    private bool ShouldGrab()
    {
        foreach (var rotation in dataReceiver.sharedData.fingerRotations)
        {
            if (rotation < grabThreshold)
                return false;
        }
        return true;
    }

    private bool ShouldRelease()
    {
        foreach (var rotation in dataReceiver.sharedData.fingerRotations)
        {
            if (rotation < releaseThreshold)
                return true;
        }
        return false;
    }

    private void GrabObject(GameObject objectToGrab)
    {
        grabbedObject = objectToGrab;
        grabbedObject.GetComponent<Rigidbody>().isKinematic = true;
        grabbedObject.transform.SetParent(handParent);
        var grabbable = grabbedObject.GetComponent<GrabbableObject>();
        if (grabbable != null)
        {
            grabbable.ApplyGrabPos(transform);
            grabbable.IsGrabbed = true;
        }
      
    }

    private void ReleaseObject()
    {
        if (grabbedObject != null)
        {
            grabbedObject.GetComponent<Rigidbody>().isKinematic = false;
            grabbedObject.GetComponent<Rigidbody>().useGravity = true;
            grabbedObject.GetComponent<Rigidbody>().velocity = velocity; // Ӧ���ͷ�ʱ���ٶ�
            grabbedObject.GetComponent<Collider>().isTrigger = false;
            grabbedObject.transform.SetParent(null);
            grabbedObject = null;
        }
    }
}
