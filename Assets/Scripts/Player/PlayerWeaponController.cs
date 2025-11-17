// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;

public delegate void SetWeaponActive();

class PlayerWeaponController : Script
{
    // ===========================================
    // Inspector variables
    // ===========================================
    public required ParticleEmitter_ muzzle;     // for gun origin.
    public required GameUIManager gameUIManager;
    
    public required Sniper sniper;
    public required Shotgun shotgun;

    public float bulletSpeed;
    public float swapWeaponCooldown = 0.2f;

    // ===========================================
    // Runtime variables
    // ===========================================
    private Gun currentlyHeldGun;

    private bool recentlySwappedWeapon = true;
    private float timeElapsed;

    private SetWeaponActive setWeaponActiveDelegate;

    protected override void init()
    {
        MapKey(Key.MouseLeft, Fire);
        ScrollCallback(SwapWeaponHandler);

        currentlyHeldGun = sniper;
        SwapWeapon(shotgun, sniper);
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        if (recentlySwappedWeapon)
        {
            if (timeElapsed < swapWeaponCooldown) {
                timeElapsed += Time.V_FixedDeltaTime();
            }
            else
            {
                recentlySwappedWeapon = false;
                timeElapsed = 0;

                // after some duration, set weapon active..
                setWeaponActiveDelegate();
            }
            
            return;
        }
    }

    private void Fire()
    {
        if (recentlySwappedWeapon) {
            return;
        }

        if(currentlyHeldGun.Fire())
        {
            muzzle.emit(30);
        }
    }

    private void SwapWeaponHandler(float scrollDelta)
    {
        if (recentlySwappedWeapon)
        {
            return;
        }

        recentlySwappedWeapon = true;
        // since we only have 2 guns, the scroll delta doesnt really matter haha..
        // in the future if we have more we need to account for it..

        if (currentlyHeldGun is Sniper)
        {
            SwapWeapon(sniper, shotgun);
        }
        else
        {
            SwapWeapon(shotgun, sniper);
        }
    }

    private void SwapWeapon(Gun from, Gun to)
    {
        currentlyHeldGun = to;
        from.gameObject.SetActive(false);

        // good old javascript syntax.
        setWeaponActiveDelegate = () =>
        {
            to.gameObject.SetActive(true);
            AudioAPI.PlaySound(gameObject, "Holster SFX");
            if(gameUIManager!= null)
                gameUIManager.SwapWeaponUI(from, to);
        };
    }
}