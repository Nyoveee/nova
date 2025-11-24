// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class EnemySpawn : Script
{
    public Vector3 StartPos; 
    public Vector3 EndPos;
    private float m_SpawnTime;
    private float duration = 2.0f;

    [SerializableField]
    private Prefab enemyPrefab;


    protected override void init()
    {
        base.init();
        m_SpawnTime = Time.V_FixedDeltaTime();
    }

    protected override void update()
    {
        Time.V_FixedDeltaTime();
    }

    // Lerp need duration
    // Script for animation
    void LerpSpawn()
    {
        float ratio =  (Time.V_FixedDeltaTime() - m_SpawnTime) / duration;

        ratio = Mathf.Clamp(ratio, 0, 1);

        if (ratio < 0)
        {
            Transform_ transform;
            transform.localPosition = Vector3.Lerp(StartPos, EndPos, ratio);
        }
        else
        {
            return;
        }
    }
}