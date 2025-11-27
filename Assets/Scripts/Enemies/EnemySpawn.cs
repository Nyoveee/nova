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
    //private Vector3 StartPos;
    private Vector3 EndPos;
    

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
        //StartPos = spawnerTransform.position;
        //StartPos.y -= yOffset;

        // testing gravity/rigid body
        EndPos = spawnerTransform.position;
        EndPos.y += yOffset;

        if (ratio < 1)
        {
            //enemyTransform.position = Vector3.Lerp(StartPos, spawnerTransform.position, ratio);
            enemyTransform.position = Vector3.Lerp(spawnerTransform.position, EndPos, ratio);
        }
        else
        {
            return;
        }

        timeElapsed += Time.V_FixedDeltaTime();

    }

    protected override void exit()
    {
         enemyTransform.gameObject.getComponent<Rigidbody_>().enable = true;
        // Aaron might need to add something here to register the spawned enemy as an entity afterwards.
    }
    // Lerp need duration
    // Script for animation
}