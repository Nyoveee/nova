// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class SpwanAgentTest : Script
{

    [SerializableField]
    public Prefab agentPrefab;
    [SerializableField]
    public GameObject camera;
    [SerializableField]
    public GameObject pt0;
    [SerializableField]
    public GameObject pt1;
    [SerializableField]
    public GameObject pt2;
    [SerializableField]
    public GameObject pt3;

    private GameObject selectObject; 
    bool isLeftClickDown = false;
    bool isRightClickDown = false;

    int count = 0;

    int slot = 0;

    //List<GameObject> agents = new List<GameObject>();
    GameObject[] agentsArr = new GameObject[4];


    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        MapKey(Key.MouseLeft, SpawnAgentLeftClick, KeyUp);
        MapKey(Key.Delete, RemoveAgent);


        MapKey(Key._0,SetZero);
        MapKey(Key._1, SetOne);
        MapKey(Key._2, SetTwo);
        MapKey(Key._3, SetThree);

        agentsArr[0] = null;
        agentsArr[1] = null;
        agentsArr[2] = null;
        agentsArr[3] = null;
    }


    public void SpawnAgentLeftClick()
    {
        //RayCastResult? result = PhysicsAPI.Raycast();

        //Ray ray = CameraAPI.getRayFromMouse();

        //Vector3 direction =  ray.origin - ray.direction ;

        //direction.Normalize();

        //RayCastResult? result = PhysicsAPI.Raycast(ray.origin, , 200f);

        //Debug.Log(ray.origin + " " + ray.direction);

        if (agentsArr[slot] != null)
        {
            Debug.Log("Slot is not Null");
        }

        if (selectObject == null || isLeftClickDown != false || agentsArr[slot] != null)
        {
            Debug.Log("SpawnAgentLeftClick");
            return;
        }


        

        isLeftClickDown = true;

        var agent = GameObject.Instantiate(agentPrefab, selectObject.transform.position , Quaternion.Identity());
        agentsArr[slot] = agent;
    }


    public void KeyUp()
    {
        isLeftClickDown = false;
    }

    //public void SpawnAgentRightClick()
    //{
    //    RayCastResult? result = PhysicsAPI.Raycast(camera.transform.position, camera.transform.front, 100f);

    //    if (result == null || isRightClickDown == false)
    //    {
    //        return;
    //    }


    //}

    public void RemoveAgent()
    {
        if (agentsArr[slot] != null)
        {

            agentsArr[slot].getComponent<NavMeshAgent_>().enable = false;
            //NavigationAPI.stopAgent(agentsArr[slot].getComponent<NavMeshAgent_>());
            GameObject.Destroy(agentsArr[slot]);
            agentsArr[slot] = null;
        }
    
    }

    public void SetZero()
    {
        slot = 0;
        selectObject = pt0;
    }

    public void SetOne()
    {
        slot = 1;
        selectObject = pt1;
    }

    public void SetTwo()
    {
        slot = 2;
        selectObject = pt2;

    }

    public void SetThree()
    { 
        slot = 3;
        selectObject = pt3;

    }

}