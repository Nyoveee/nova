// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using System.Runtime.Serialization;

class UltimateController : Script
{
    // ==============================================
    // Serialized Fields
    // ==============================================
    public Prefab ultimate;
    public Transform_ camera;
    
    public GameObject ultimatePose;
    public MeshRenderer_ originalGun;
    public PlayerControllerV2 playerController;
    public PlayerWeaponController playerWeaponController;

    public Sequence_ sequence;

    public float projectileSpeed = 20f;

    public float timeScaleSlow = 0.15f;
    public float timeScaleLerpDuration = 0.5f;

    public float vignetteMultiplier = 0.6f;
    public float vigenetteDuration = 1.0f;
    public float vigenetteFadeOutDuration = 0.2f;

    // ==============================================
    // Runtime variables
    // ==============================================
    private Rigidbody_ rigidbody;
    private bool isCasting = false;

    private bool isSlowingDownTime = false;
    private float timeScaleTimeElapsed = 0f;

    private bool isAnimatingVignette = false;
    private bool isAnimatingVignetteFadeOut = false;
    private float vignetteTimeElapsed = 0f;


    // This function is first invoked when game starts.
    protected override void init()
    {
        MapKey(Key.F, BeginUltimateSequence);
        rigidbody = getComponent<Rigidbody_>();
        RendererAPI.toPostProcess = true;
        RendererAPI.vignette = 0f;
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        handleVignetteLerp();
        handleTimeScaleLerp();
    }

    private void handleVignetteLerp()
    {
        if (isAnimatingVignette && vignetteTimeElapsed < vigenetteDuration)
        {
            float interval = vignetteTimeElapsed / vigenetteDuration;

            RendererAPI.vignette = Mathf.Interpolate(0f, vignetteMultiplier, interval, 1f);
            vignetteTimeElapsed += Time.V_DeltaTime_Unscaled();
        }

        if (isAnimatingVignetteFadeOut && vignetteTimeElapsed < vigenetteFadeOutDuration)
        {
            float interval = vignetteTimeElapsed / vigenetteFadeOutDuration;

            RendererAPI.vignette = Mathf.Interpolate(vignetteMultiplier, 0f, interval, 1f);
            vignetteTimeElapsed += Time.V_DeltaTime_Unscaled();

            if(vignetteTimeElapsed > vigenetteFadeOutDuration)
            {
                isAnimatingVignetteFadeOut = false;
            }
        }
    }
    private void handleTimeScaleLerp()
    {
        if (isSlowingDownTime && timeScaleTimeElapsed < timeScaleLerpDuration)
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

        playerWeaponController.weaponControlStates = PlayerWeaponController.WeaponControlStates.Busy;

        isCasting = true;
        rigidbody.enable = false;
        playerController.ToEnable = false;

        Invoke(() =>
        {
            originalGun.enable = false;
            ultimatePose.SetActive(true);
        }, 0.2f);

        isAnimatingVignette = true;
        vignetteTimeElapsed = 0;

        sequence.play();
    }

    public void EndUltimateSequence()
    {
        playerWeaponController.weaponControlStates = PlayerWeaponController.WeaponControlStates.WeaponFree;
        playerWeaponController.ResetGunPosition();

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
        isAnimatingVignette = false;
        isAnimatingVignetteFadeOut = true;
        vignetteTimeElapsed = 0f;

        GameObject projectile = Instantiate(ultimate, ultimatePose.transform.position);

        projectile.getComponent<Rigidbody_>().SetVelocity(camera.front * projectileSpeed);
    }
}