// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using System.Collections.Generic;

class SummonerTest : Script
{
    List<PathfindingCube> gamobjectList = new List<PathfindingCube>();

    [SerializableField]
    public Prefab cubePrefab;

    // This function is first invoked when game starts.
    protected override void init()
    {}

    // This function is invoked every fixed update.
    protected override void update()
    {
        // insta
        //ray cast to a position to spawn cube
        Ray ray = CameraAPI.getRayFromMouse();
        RayCastResult? result = PhysicsAPI.Raycast(ray, 1000f);

        if (result != null)
        {
            //Debug.Log("ray hit at " + result.Value.point + ", hitting entity " + result.Value.entity);
            Instantiate(cubePrefab, result.Value.point,Quaternion.identity);

        }
    }

}