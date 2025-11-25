// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class EnemySpawn : Script
{
    private Transform_ enemyTransform;
    private Transform_ spawnerTransform;
    public float yOffset;
    public float duration;
    private float timeElapsed = 0f;
    

    [SerializableField]
    private Prefab enemyPrefab;

    protected override void init()
    {
        spawnerTransform = getComponent<Transform_>();
        enemyTransform = Instantiate(enemyPrefab).transform;
    }

    protected override void update()
    {
        float ratio = timeElapsed / duration;
        Vector3 EndPos = spawnerTransform.position;
        EndPos.y += yOffset;

        if (ratio < 1)
        {
            enemyTransform.position = Vector3.Lerp(spawnerTransform.position, EndPos, ratio);
        }
        else
        {
            return;
        }

        timeElapsed += Time.V_FixedDeltaTime();

        //Debug.Log("balls");
    }

    // Lerp need duration
    // Script for animation
}