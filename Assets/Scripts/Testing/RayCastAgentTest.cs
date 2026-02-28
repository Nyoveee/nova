// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using System.Data.SqlTypes;

class RayCastAgentTest : Script
{
    // This function is invoked once when gameobject is active.

    bool isLeftClickDown = false;

    public Prefab? prefab = null;
    protected override void init()
    {
        MapKey(Key.MouseLeft, SpawnAgentLeftClick, KeyUp);

    }

    // This function is invoked every update.
    protected override void update()
    {}


    private void SpawnAgentLeftClick()
    { 
        isLeftClickDown = true;


        Ray ray = CameraAPI.getRayFromMouse();

        //Vector3 direction = ray.origin - ray.direction;

        //direction.Normalize();

        RayCastResult? result = PhysicsAPI.Raycast(ray.origin, ray.direction, 200f);

        if (result != null)
        {
            GameObject agent = Instantiate(prefab, result.Value.point, Quaternion.Identity());

            Vector3? newPos = NavigationAPI.SampleNavMeshPosition("Humanoid", result.Value.point, new Vector3(1f, 100f, 1f)  );

            if (newPos != null)
            {
                //agent.transform.position = newPos.Value;
                agent.getComponent<NavMeshAgent_>().Warp(newPos.Value);
                Debug.Log("Enter");
                Debug.Log("Click Value: " + result.Value.point);
            }

           // Debug.Log("Click Value: " + result.Value.point);
        }
    }


    private void KeyUp()
    {
        isLeftClickDown = false;
    }

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}