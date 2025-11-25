// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class EnemySpawn : Script
{
    private Transform_ enemyTransform;
    public Vector3 StartPos; 
    public Vector3 EndPos;
    private float timeElapsed = 0f;
    private float duration = 2f;

    [SerializableField]
    private Prefab enemyPrefab;

    protected override void init()
    {
        enemyTransform = Instantiate(enemyPrefab).transform;
    }

    protected override void update()
    {
        float ratio = timeElapsed / duration;

        // ratio = Mathf.Clamp(ratio, 0, 1);

        if (ratio < 1)
        {
            enemyTransform.position = Vector3.Lerp(StartPos, EndPos, ratio);
        }
        else
        {
            return;
        }

        timeElapsed += Time.V_FixedDeltaTime();
    }

    // Lerp need duration
    // Script for animation
}