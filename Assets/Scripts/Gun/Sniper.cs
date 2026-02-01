// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;

class Sniper : Gun
{
    /***********************************************************
        Inspector Variables
    ***********************************************************/
    public required Transform_ camera;
    public required float cooldown = 1f;
    public required float recoilDuration = 0.2f;
    public required float range = 1000f;
    public required float damage = 40f;
    [SerializableField]
    private List<Audio> fireSFX;
    [SerializableField]
    private Audio errorAmmoSFX;
    [SerializableField]
    private List<Audio> hitMarkerSFX;


    /***********************************************************
        Components
    ***********************************************************/
    private AudioComponent_ audioComponent;

    /***********************************************************
        Runtime variables..
    ***********************************************************/
    private float timeElapsed = 0f;
    private bool onCooldown = false;
    private bool isRecoiling = false;
    private Vector3 startPosition;
    private Vector3 backPosition;

    protected override void init()
    {
        base.init();
        audioComponent = getComponent<AudioComponent_>();
    }
    protected override void update()
    {
        if (!onCooldown)
        {
            return;
        }

        // Handle gun recoil lerping..
        if(timeElapsed < recoilDuration)
        {
            gameObject.transform.localPosition = Vector3.Lerp(startPosition, backPosition, Mathf.Pow(timeElapsed / recoilDuration, 0.3f));
        }
        else if (timeElapsed < recoilDuration * 2)
        {
            gameObject.transform.localPosition = Vector3.Lerp(backPosition, startPosition, Mathf.Pow((timeElapsed - recoilDuration) / recoilDuration, (1 / 0.3f)));
        }
        else if (isRecoiling)
        {
            isRecoiling = false;
            gameObject.transform.localPosition = startPosition;
        }

        if (timeElapsed > cooldown)
        {
            timeElapsed = 0f;
            onCooldown = false;
        }
        else
        {
            timeElapsed += Time.V_DeltaTime();
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
        isRecoiling = true;
        CurrentAmmo--;

        audioComponent.PlayRandomSound(fireSFX);

        RayCastFire(camera.position, camera.front, range, damage);

        startPosition = gameObject.transform.localPosition;
        backPosition = startPosition - (Vector3.Front() * 2f);
        
        return true;
    }
}