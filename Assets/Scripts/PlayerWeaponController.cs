// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;

class PlayerWeaponController : Script
{
    public required Prefab bulletPrefab;
    public required Transform_ gunHolder;   // for gun origin.
    public required Transform_ camera;      // for gun path
    public required float bulletSpeed = 10f;

    protected override void init()
    {
        MapKey(Key.MouseLeft, SpawnBullet);
    }

    // This function is invoked every fixed update.
    protected override void update()
    {}

    private void SpawnBullet()
    {
        GameObject bullet = ObjectAPI.Instantiate(bulletPrefab, gunHolder.position, null);

        if (bullet != null) {
            bullet.getComponent<Rigidbody_>().SetVelocity(camera.front * bulletSpeed);
        }
    }
}