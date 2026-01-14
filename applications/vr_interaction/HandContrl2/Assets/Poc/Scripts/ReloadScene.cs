using UnityEngine;
using UnityEngine.SceneManagement;

public class ReloadScene : MonoBehaviour
{
    void Update()
    {
        // ����Ƿ�����Q��
        if (Input.GetKeyDown(KeyCode.Q))
        {
            ReloadCurrentScene();
        }
        
    }

    void ReloadCurrentScene()
    {
        // ��ȡ��ǰ�����������
        string sceneName = SceneManager.GetActiveScene().name;
        // ���¼��ص�ǰ�����
        SceneManager.LoadScene(sceneName);
    }
}
