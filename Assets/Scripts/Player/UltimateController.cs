// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using System.Runtime.Serialization;
using Windows.Media.PlayTo;

class UltimateController : Script
{
    // ==============================================
    // Serialized Fields
    // ==============================================
    public Prefab ultimate;
    public Transform_ camera;
    
    public GameObject ultimatePose;
    public GameObject muzzle;
    public MeshRenderer_ originalGun;
    public PlayerController_V2 playerController;
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
    private VignetteController vignetteController;
    // ===========================================
    // Components
    // ===========================================
    private AudioComponent_? audioComponent;

    [SerializableField]
    private Audio ultSFX;

    // This function is first invoked when game starts.
    protected override void init()
    {
        MapKey(Key.F, BeginUltimateSequence);
        rigidbody = getComponent<Rigidbody_>();
        vignetteController = GameObject.FindWithTag("Game UI Manager")?.getScript<VignetteController>();
        audioComponent = getComponent<AudioComponent_>();
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        handleTimeScaleLerp();
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
        if (isCasting) 
        {
            return;
        }

        if (playerWeaponController.currentlyHeldGun.CurrentSp != playerWeaponController.currentlyHeldGun.MaxSp)
        {
            return;
        }

        playerWeaponController.currentlyHeldGun.CurrentSp = 0;

        audioComponent.PlaySound(ultSFX);
        
        playerWeaponController.weaponControlStates = PlayerWeaponController.WeaponControlStates.Busy;

        isCasting = true;

        //rigidbody.enable = false;
        playerController.GravityFreeze(true);
        playerController.PositionFreeze(true);
        playerController.playerMoveStates = PlayerMoveStates.Disabled;
        vignetteController.TriggerVignetteFadeIn(vignetteMultiplier, vigenetteDuration,new Colour(0f, 0f, 0f));
        Invoke(() =>
        {
            originalGun.enable = false;
            ultimatePose.SetActive(true);
        }, 0.2f);

        
        playerController.movementIsEnabled = false;
        playerController.SetIframes(true);
        sequence.play();

    }

    public void EndUltimateSequence()
    {
        playerWeaponController.weaponControlStates = PlayerWeaponController.WeaponControlStates.WeaponFree;
        playerWeaponController.ResetGunPosition();
        playerController.GravityFreeze(false);
        playerController.PositionFreeze(false);

        isCasting = false;
        originalGun.enable = true;
        playerController.playerMoveStates = PlayerMoveStates.GroundedMovement;
        //rigidbody.enable = true;
        Time.timeScale = 1f;
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


        GameObject projectile = Instantiate(ultimate, muzzle.transform.position);

        string[] mask = { "Enemy_HurtSpot", "NonMoving", "Floor" };
        // Raycast..
        RayCastResult? result = PhysicsAPI.Raycast(camera.position, camera.front, 500f, mask);

        Vector3 direction = muzzle.transform.front;

        if (result != null)
        {
            direction = result.Value.point - muzzle.transform.position;            
        }

        direction.Normalize();
        projectile.getComponent<Rigidbody_>().SetVelocity(direction * projectileSpeed);
        vignetteController.TriggerVignetteFadeOut(vignetteMultiplier, vigenetteDuration, new Colour(0f, 0f, 0f));
        playerController.movementIsEnabled = true;
        playerController.SetIframes(false);
    }
}