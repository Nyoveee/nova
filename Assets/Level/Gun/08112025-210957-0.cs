// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

public abstract class Gun : Script
{
    public int currentAmmo;
    public int maxAmmo;
    
    protected override void init()
    {
        maxAmmo = currentAmmo;
    }

    public abstract void Fire();

    public RayCastResult? RayCast(Transform_ camera, float range)
    {
        return PhysicsAPI.Raycast(camera.position, camera.front, range, gameObject);
    }
}