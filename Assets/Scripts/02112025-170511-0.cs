// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;

class PlayerWeaponController : Script
{
    public Prefab prefab;
    public Texture texture; //asd as BLUE ARCHIVE S asd sds
    public Material material;
    public Model model;

    // adasdasd
    // This function is first invoked when game starts.
    protected override void init()
    {
        Debug.Log(prefab);
        Debug.Log(texture);
        Debug.Log(material);
        Debug.Log(model);
    }

    // This function is invoked every fixed update.
    protected override void update()
    {}

}