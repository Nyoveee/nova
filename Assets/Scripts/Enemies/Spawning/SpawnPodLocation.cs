// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class SpawnPodLocation : Script
{
    public List<Prefab> enemies;

    protected override void init()
    {
        // we use a mesh renderer to visualise where they will be spawning..
        MeshRenderer_ meshRenderer = getComponent<MeshRenderer_>();

        if(meshRenderer != null)
        {
            meshRenderer.enable = false;
        }
    }
}