using System.Collections.Generic;
using UnityEngine;

public class SpawnController : MonoBehaviour
{
    [SerializeField]
    private Vector3 spwanPosition;
    [SerializeField]
    private Vector3 spwanRotation;
    public List<GameObject> prefabs; // ��ץȡ�����Ԥ�����б�
    public Transform spawnPoint; // �������ɵ�λ��

    private GameObject currentObject; // ��ǰ�����е�����
    private int currentIndex = 0; // ��ǰѡ���Ԥ��������

    void Update()
    {
        if (Input.GetKeyDown(KeyCode.LeftArrow))
        {
            // �л���ǰһ��Ԥ����
            ChangePrefab(-1);
        }
        else if (Input.GetKeyDown(KeyCode.RightArrow))
        {
            // �л�����һ��Ԥ����
            ChangePrefab(1);
        }
        
    }

    private void ChangePrefab(int direction)
    {
        currentIndex += direction;

        // ѭ���б�
        if (currentIndex >= prefabs.Count) currentIndex = 0;
        if (currentIndex < 0) currentIndex = prefabs.Count - 1;

        // ��鵱ǰ�����е������Ƿ��Ѿ���ʰȡ
        if (currentObject != null && !currentObject.GetComponent<GrabbableObject>().IsGrabbed)
        {
            Destroy(currentObject); // �����ǰ����δ��ʰȡ����ɾ��
        }

        // �����µ�����
        SpawnObject();
    }

    private void SpawnObject()
    {
        if (prefabs[currentIndex] != null)
        {
            currentObject = Instantiate(prefabs[currentIndex], spwanPosition, Quaternion.Euler(spwanRotation));
            currentObject.SetActive(true);
        }
    }
}
