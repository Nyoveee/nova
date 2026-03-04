// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class Waypoint_Test : Script
{

    public GameObject? wayPointAgent = null;

    private Waypoint_Agent agentScript;




    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        MapKey(Key.MouseLeft, MovetoLeftClick);

        if (wayPointAgent != null)
        {
          agentScript =  wayPointAgent.getScript<Waypoint_Agent>();
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


    public void MovetoLeftClick()
    {
        Ray ray = CameraAPI.getRayFromMouse();

        //Vector3 direction = ray.origin - ray.direction;

        //direction.Normalize();

        RayCastResult? result = PhysicsAPI.Raycast(ray.origin, ray.direction, 200f);

        if (result != null)
        {
            ///Vector3? newPos = NavigationAPI.SampleNavMeshPosition("Humanoid", result.Value.point, new Vector3(0.01f, 1f, 0.01f));

            //if (newPos != null)
            //{
                agentScript.SetNewPosition(result.Value.point);
            //}
        }


    }
}