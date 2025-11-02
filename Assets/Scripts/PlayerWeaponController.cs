// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;

class PlayerWeaponController : Script
{
    public Prefab bulletPrefab;

    protected override void init()
    {
        Input.MapKey(Key.MouseLeft, SpawnBullet);
    }

    // This function is invoked every fixed update.
    protected override void update()
    {}

    private void SpawnBullet()
    {
        // ObjectAPI.Instantiate(bulletPrefab);
    }
}