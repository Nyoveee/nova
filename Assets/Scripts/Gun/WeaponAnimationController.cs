// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class WeaponAnimationController : Script
{
    // ===========================================
    // References
    // ===========================================
    [SerializableField]
    private PlayerController_V2? playerMovementController = null;
    [SerializableField]
    private PlayerWeaponController? playerWeaponController = null;

    // ===========================================
    // Parameters
    // ===========================================
    [SerializableField]
    public float swaySpeed = 0.0f;
    [SerializableField]
    public float swayOffset = 0.1f;

    // ===========================================
    // References
    // ===========================================
    private Sequence_ gunSequence;
    private Transform_ weaponTransform;
    private Vector3 baseWeaponPosition;

    // ===========================================
    // Runtime Variables
    // ===========================================
    private float recoilDuration = 0.0f;
    private float recoilTimeElapsed = 0.0f;
    private WeaponAnimationStates weaponAnimationStates = WeaponAnimationStates.Idle;
    
    //private float currentSwayOffset = 0.0f;
    private float lerpSwayVariable = 0.0f;
    private Vector3 currentSwayDirection = Vector3.Zero();
    private Vector3 oldSwayPositionChange = Vector3.Zero();

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}



    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        //gameObject.getComponent<Sequence_>();
        //gameObject.ga

        if (playerMovementController == null)
        { 
            Debug.LogError("WeaponAnimationController: playerMovementController is null!");
        }

        gunSequence = gameObject.getComponent<Sequence_>();
        baseWeaponPosition = gameObject.transform.localPosition;

    }

    // This function is invoked every update.
    protected override void update()
    {
        //playerWeaponController.

        switch (weaponAnimationStates)
        { 
                case WeaponAnimationStates.Idle:
                {
                    if (playerMovementController.playerMoveStates == PlayerMoveStates.GroundedMovement && 
                        playerMovementController.GetDirectionVector() != Vector3.Zero() &&
                        playerWeaponController.weaponControlStates == PlayerWeaponController.WeaponControlStates.WeaponFree)
                    {
                        StartSwayAnimation();
                        weaponAnimationStates = WeaponAnimationStates.Swaying;
                    }


                }
                break;
            case WeaponAnimationStates.Swaying:
                {
                    if (playerMovementController.playerMoveStates != PlayerMoveStates.GroundedMovement || 
                        playerMovementController.GetDirectionVector() == Vector3.Zero() ||
                        playerWeaponController.weaponControlStates != PlayerWeaponController.WeaponControlStates.WeaponFree)
                    {
                        StopSwayAnimation();
                        weaponAnimationStates = WeaponAnimationStates.Idle;

                    }
                    else 
                    {
                       SwayingAnimation();
                    }

                }
                break;
            case WeaponAnimationStates.Recoil:
                { 
                    recoilTimeElapsed += Time.V_DeltaTime();
                    if (recoilTimeElapsed > recoilDuration)
                    {
                        recoilTimeElapsed = 0.0f;
                        weaponAnimationStates = WeaponAnimationStates.Idle;
                    }

                }
                break;
            case WeaponAnimationStates.ThrowSurge:
                { 
                }
                break;
        
        }


    }

    // ===========================================
    // Public Functions
    // ===========================================
    public void PlayRecoilAnimation(float recoilDuration) { 
    
        if(weaponAnimationStates == WeaponAnimationStates.Swaying) {
            gameObject.transform.localPosition = baseWeaponPosition;
        }


        gunSequence.play();
        weaponAnimationStates = WeaponAnimationStates.Recoil;
        this.recoilDuration = recoilDuration;
        recoilTimeElapsed = 0.0f;
    }


    public void StartSwayAnimation()
    { 
        //recoil has priority
        if(weaponAnimationStates == WeaponAnimationStates.Recoil) {
            return;
        }

        lerpSwayVariable = 0.0f;
        currentSwayDirection.x = -1;
        oldSwayPositionChange = baseWeaponPosition;
        weaponAnimationStates = WeaponAnimationStates.Swaying;
    
    }

    public void StopSwayAnimation()
    {
        //recoil has priority
        lerpSwayVariable = 0.0f;
        currentSwayDirection.x = 0;
        gameObject.transform.localPosition = baseWeaponPosition;
        weaponAnimationStates = WeaponAnimationStates.Idle;

    }


    public void EnableThrow()
    {
        playerWeaponController.EnableWeaponArm();

        Debug.Log("EnableThrow!!!");
    }

    public void DisableThrow()
    {
        Debug.Log("DisableThrow!!!");
        playerWeaponController.DisableWeaponArm();
    }


    // ===========================================
    // Private Functions
    // ===========================================

    void SwayingAnimation()
    {
        lerpSwayVariable += Time.V_DeltaTime();

        //sway left
        if (currentSwayDirection.x < 0)
        {

            gameObject.transform.localPosition = Vector3.Lerp(oldSwayPositionChange , baseWeaponPosition + currentSwayDirection * swayOffset, lerpSwayVariable / swaySpeed);
            if (lerpSwayVariable > swaySpeed)
            {
                currentSwayDirection.x = 1;
                lerpSwayVariable = 0.0f;
                oldSwayPositionChange= gameObject.transform.localPosition;
            }


        }
        else if( currentSwayDirection.x > 0)
        {
            gameObject.transform.localPosition = Vector3.Lerp(oldSwayPositionChange, baseWeaponPosition + currentSwayDirection * swayOffset, lerpSwayVariable / swaySpeed);
            if (lerpSwayVariable > swaySpeed)
            {
                currentSwayDirection.x = -1;
                lerpSwayVariable = 0.0f;
                oldSwayPositionChange = gameObject.transform.localPosition;
            }
        }
    
    
    }

    


    enum WeaponAnimationStates 
    { 
        Idle,
        Swaying,
        Recoil,
        ThrowSurge,
    
    }

}