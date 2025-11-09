// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class Shotgun : Gun
{
    // Inspector variables
    public required Transform_ camera;
    public required float cooldown = 1f;
    public required float range = 1000f;
    public required float damage = 40f;

    // Runtime variables
    private float timeElapsed = 0f;
    private bool onCooldown = false;

    protected override void update()
    {
        if (!onCooldown)
        {
            if (timeElapsed > cooldown)
            {
                timeElapsed = 0f;
                onCooldown = false;
            }
            else
            {
                timeElapsed += Time.V_FixedDeltaTime();
            }
        }
    }

    // Returns true if the gun is off cooldown and a fire happens.
    public override bool Fire()
    {
        if (onCooldown)
        {
            return false;
        }

        onCooldown = true;

        RayCastFire(camera.position, camera.front, range, damage);

        return true;
    }
}