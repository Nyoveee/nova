// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;

class PlayerWeaponController : Script
{
    // ===========================================
    // Inspector variables
    // ===========================================
    public required ParticleEmitter_ muzzle;     // for gun origin.

    public required Sniper sniper;
    public required Shotgun shotgun;

    public float bulletSpeed;

    // ===========================================
    // Runtime variables
    // ===========================================
    private Gun currentlyHeldGun;

    protected override void init()
    {
        MapKey(Key.MouseLeft, Fire);
        currentlyHeldGun = sniper;
    }

    // This function is invoked every fixed update.
    protected override void update()
    {}

    private void Fire()
    {
        if(currentlyHeldGun.Fire())
        {
            muzzle.emit(30);
        }
    }
}