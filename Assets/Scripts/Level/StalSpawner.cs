// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class StalSpawner : Script
{
    [SerializableField]
    private Prefab stalactite1;
    [SerializableField]
    private Prefab stalactite2;
    [SerializableField]
    private Prefab stalactite3;
    [SerializableField]
    private GameObject scrollParent;

    [SerializableField]
    private GameObject boundsMin;
    [SerializableField]
    private GameObject boundsMax;
    [SerializableField] 
    private int spawnCount = 10;

    private Prefab[] prefabs;
    private System.Random rng;

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        string[] ceilingMask = { "Wall" };

        prefabs = new Prefab[] { stalactite1, stalactite2, stalactite3 };
        rng = new System.Random();

        Vector3 min = boundsMin.transform.position;
        Vector3 max = boundsMax.transform.position;

        for (int i = 0; i < spawnCount; i++)
        {
            float x = min.x + (float)(rng.NextDouble() * (max.x - min.x));
            float z = min.z + (float)(rng.NextDouble() * (max.z - min.z));

            Vector3 rayOrigin = new Vector3(x, min.y, z);
            var result = PhysicsAPI.Raycast(rayOrigin, Vector3.Up(), 1000, ceilingMask);

            float y;
            if (result != null)
                y = result.Value.point.y;
            else
                y = max.y;

            Vector3 parentPos = scrollParent.transform.position;

            Prefab chosen = prefabs[rng.Next(0, prefabs.Length)];
            Instantiate(chosen, new Vector3(x - parentPos.x, y - parentPos.y, z - parentPos.z), Quaternion.AngleAxis(0f, new Vector3(0f, 1f, 0f)), scrollParent);
        }

    }

    // This function is invoked every update.
    protected override void update()
    {}

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}