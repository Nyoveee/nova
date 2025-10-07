// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using System.Net.Http.Headers;

class PathFinding : Script
{
    public UInt32 agent;
    
    // This function is first invoked when game starts.
    protected override void init()
    {
        Input.MapKey(Key.MouseLeft, onMouseClick, onMouseRelease);
    }

    // This function is invoked every fixed update.
    protected override void update()
    {}

    private void onMouseClick() 
    {
        Ray ray = CameraAPI.getRayFromMouse();
        RayCastResult? result = PhysicsAPI.Raycast(ray, 1000f);

        if(result != null)
        {
            Debug.Print("ray hit at " + result.Value.point + ", hitting entity " + result.Value.entity);
            NavigationAPI.setDestination(entityID, result.Value.point);
        }
        else
        {
            Debug.Print("ray cast miss!");
        }
    }

    private void onMouseRelease()
    {

    }
}