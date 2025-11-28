// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class UltimateController : Script
{
    // ==============================================
    // Serialized Fields
    // ==============================================
    public Prefab ultimate;
    public Transform_ camera;
    
    public GameObject ultimatePose;
    public MeshRenderer_ originalGun;
    public PlayerController playerController;

    public Sequence_ sequence;

    public float projectileSpeed = 20f;

    public float timeScaleSlow = 0.15f;
    public float timeScaleLerpDuration = 0.5f;
    // ==============================================
    // Runtime variables
    // ==============================================
    private Rigidbody_ rigidbody;
    private bool isCasting = false;

    private bool isSlowingDownTime = false;
    private float timeScaleTimeElapsed = 0f;

    // This function is first invoked when game starts.
    protected override void init()
    {
        MapKey(Key.E, BeginUltimateSequence);
        rigidbody = getComponent<Rigidbody_>();
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        if(isSlowingDownTime && timeScaleTimeElapsed < timeScaleLerpDuration)
        {
            float interval = timeScaleTimeElapsed / timeScaleLerpDuration;

            Time.timeScale = Mathf.Interpolate(1f, timeScaleSlow, interval, 1f);
            timeScaleTimeElapsed += Time.V_DeltaTime_Unscaled();
        }
    }

    private void BeginUltimateSequence()
    {
        if (isCasting) {
            return;
        }

        isCasting = true;
        rigidbody.enable = false;
        playerController.ToEnable = false;

        Invoke(() =>
        {
            originalGun.enable = false;
            ultimatePose.SetActive(true);
        }, 0.2f);
        
        sequence.play();
    }

    public void EndUltimateSequence()
    {
        isCasting = false;
        originalGun.enable = true;
        playerController.ToEnable = true;
        rigidbody.enable = true;
        ultimatePose.SetActive(false);
    }

    public void TimeScaleSlow()
    {
        timeScaleTimeElapsed = 0f;
        isSlowingDownTime = true;
    }

    // Creates the ultimate projectile
    public void CastUltimate()
    {
        Time.timeScale = 1.0f;
        isSlowingDownTime = false;

        GameObject projectile = Instantiate(ultimate, ultimatePose.transform.position);

        projectile.getComponent<Rigidbody_>().SetVelocity(camera.front * projectileSpeed);
    }
}