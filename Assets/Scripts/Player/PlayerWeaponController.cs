// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;
using System;
using static PlayerWeaponController;

public delegate void SetWeaponActive();

class PlayerWeaponController : Script
{
    // ===========================================
    // Inspector variables
    // ===========================================
    public required ParticleEmitter_ muzzle;     // for gun origin.
    public required Transform_ gunHolder;
    public required Transform_ throwPosition;
    public required Transform_ gunPosition;
    public required Transform_ playerCamera;
    public required Sniper sniper;
    public required MeshRenderer_ sniperMesh;
    public required GameObject playerCollider;
    public required GameObject playerArmGun;
    public required Prefab thrownRiflePrefab;
    public required Prefab ammoTrailPrefab;
    public required Sequence_ weaponOffset;

    public float armingTime = 0.3f;
    public float bulletSpeed;
    public float swapWeaponCooldown = 0.2f;

    public float glowDownDuration = 1f;
    public float peakGlowStrength = 1.5f;
    public float noAmmoGlowStrength = 0.6f;
    public float ammoGlowScalePower = 2f;


    // ===========================================
    // Components
    // ===========================================
    private AudioComponent_ audioComponent;



    // ===========================================
    // Runtime variables
    // ===========================================
    private GameUIManager gameUIManager;
    public Gun currentlyHeldGun;
    private float timeElapsed;
    private float armTimeElapsed = 0f;
    private bool isArmingDisabled = false; //required by animation controller disable animations while other animation are playing
    private bool isArmingRequest = false;
    private bool isShootingDisabled = false;

    public WeaponControlStates weaponControlStates;

    private float glowChangeDuration = 0f;
    private float glowTimeElapsed = 0f;
    private float initialGlowStrength;
    private float finalGlowStrength;

    const int SNIPER_MATERIAL_MATERIAL_INDEX = 0;
    const int SNIPER_BARREL_MATERIAL_INDEX = 1;

    private float lerpVariable = 0;

    const string ThrowAnimationName = "PlayerHandThrowAnimation";
    const string IdleAnimationName = "PlayerHandIdleAnimation_Gun";
    const string FiringAnimationName = "PlayerHandFiringAnimation_Gun";
    const string RetrieveAnimationName = "PlayerHandRetrieveAnimation";

    public enum WeaponControlStates
    {
        Busy,
        WeaponFree,
        ArmingThrow,
        DisarmingFree,
        ThrowReady,
        AwaitWeaponReturn,
        WeaponRecieve,
    }

    // private SetWeaponActive setWeaponActiveDelegate;

    protected override void awake()
    {
        gameUIManager = GameObject.FindWithTag("Game UI Manager")?.getScript<GameUIManager>();
        currentlyHeldGun = sniper;
        weaponControlStates = WeaponControlStates.WeaponFree;
        audioComponent = getComponent<AudioComponent_>();
    }

    protected override void init()
    {
        MapKey(Key.MouseLeft, Fire);
        MapKey(Key.MouseRight, Arming, Disarming);

        //ScrollCallback(SwapWeaponHandler);
    }

    // This function is invoked every update.
    protected override void update()
    {
        // Regardless of weapon state, we handle the glow VFX of sniper..
        handleWeaponGlow();

        lerpVariable = Math.Clamp(lerpVariable, 0, 1);
        gunHolder.localPosition = Vector3.Lerp(gunPosition.localPosition, throwPosition.localPosition, lerpVariable);

        switch (weaponControlStates)
        {
            case WeaponControlStates.Busy:
                { //anystate you want to lock weaponcontrols

                }
                break;
            case WeaponControlStates.WeaponFree:
                {
                    if (currentlyHeldGun.CurrentAmmo != 0 && isArmingDisabled == false)
                    {

                    }
                    else if (currentlyHeldGun.CurrentAmmo <= 0 && isArmingDisabled == false)
                    {
                        SetArmingState();
                    }

                    //player is trying to arm while weapon is busy with animation, now animation is over play arming
                    if (isArmingRequest == true && isArmingDisabled == false)
                    {
                        SetArmingState();
                        isArmingRequest = false;
                    }

                }
                break;
            case WeaponControlStates.ArmingThrow:
                {

                }
                break;
            case WeaponControlStates.DisarmingFree:
                {
                    // Transition to idle mode..
                    if (playerArmGun.getComponent<Animator_>().GetTimeElapsed() == 0)
                    {
                        weaponControlStates = WeaponControlStates.WeaponFree;
                        playerArmGun.getComponent<Animator_>().speedMultiplier = 1f;
                        playerArmGun.getComponent<Animator_>().PlayAnimation(IdleAnimationName);
                    }
                }
                break;
            case WeaponControlStates.ThrowReady:
                { }
                break;
            case WeaponControlStates.AwaitWeaponReturn:
                { }
                break;
            case WeaponControlStates.WeaponRecieve:
                break;
        }
    }

    // There are 3 parameters guiding the weapon glow
    // Initial glow strength, final glow strength, and current time.

    // The purpose of this function is to lerp between the 2 based on current time, and set the material's glow
    // strength accordingly.
    private void handleWeaponGlow()
    {
        if (glowTimeElapsed < glowChangeDuration)
        {
            float interval = glowTimeElapsed / glowChangeDuration;
            float glowIntensity = Mathf.Interpolate(initialGlowStrength, finalGlowStrength, interval, 1);

            sniperMesh.setMaterialFloat(SNIPER_BARREL_MATERIAL_INDEX, "emissiveStrength", glowIntensity);
            sniperMesh.setMaterialFloat(SNIPER_MATERIAL_MATERIAL_INDEX, "emissiveStrength", glowIntensity);
        }

        glowTimeElapsed += Time.V_DeltaTime();
    }

    private void Arming()
    {
        if (currentlyHeldGun.CurrentAmmo != 0 && (weaponControlStates == WeaponControlStates.WeaponFree || weaponControlStates == WeaponControlStates.DisarmingFree) && isArmingDisabled == false)
        {
            SetArmingState();
        }

        if (isArmingDisabled)
        {
            isArmingRequest = true;
        }
    }

    private void SetArmingState()
    {
        weaponControlStates = WeaponControlStates.ArmingThrow;

        playerArmGun.getComponent<Animator_>().speedMultiplier = 1f;
        playerArmGun.getComponent<Animator_>().PlayAnimation(ThrowAnimationName);

        weaponOffset.speedMultiplier = 1f;
        weaponOffset.play();
    }

    private void Disarming()
    {
        if (currentlyHeldGun.CurrentAmmo != 0 && (weaponControlStates == WeaponControlStates.ArmingThrow || weaponControlStates == WeaponControlStates.ThrowReady))
        {
            weaponControlStates = WeaponControlStates.DisarmingFree;
            isArmingRequest = false;

            playerArmGun.getComponent<Animator_>().speedMultiplier = -1f;

            weaponOffset.speedMultiplier = -1f;
            weaponOffset.play();
        }
    }

    private void AnimateGunGlow()
    {
        // Specify lerp properties..
        glowChangeDuration = glowDownDuration;
        glowTimeElapsed = 0;

        initialGlowStrength = peakGlowStrength;

        // The final glow strength scales with how low the ammo count is..
        finalGlowStrength = noAmmoGlowStrength * Mathf.Pow(1f - (float)currentlyHeldGun.CurrentAmmo / (float)currentlyHeldGun.MaxAmmo, ammoGlowScalePower);
        sniperMesh.setMaterialFloat(SNIPER_BARREL_MATERIAL_INDEX, "emissiveStrength", finalGlowStrength);
        sniperMesh.setMaterialFloat(SNIPER_MATERIAL_MATERIAL_INDEX, "emissiveStrength", finalGlowStrength);
    }

    private void Fire()
    {
        if (isShootingDisabled)
            return;

        if (weaponControlStates == WeaponControlStates.WeaponFree && currentlyHeldGun.Fire())
        {
            playerArmGun.getComponent<Animator_>().PlayAnimation(FiringAnimationName);

            // ---------------------------------------------------------------
            // The moment this gun fires, the brightness of the glow spikes.
            sniperMesh.setMaterialFloat(SNIPER_BARREL_MATERIAL_INDEX, "emissiveStrength", peakGlowStrength);
            sniperMesh.setMaterialFloat(SNIPER_MATERIAL_MATERIAL_INDEX, "emissiveStrength", peakGlowStrength);

            AnimateGunGlow();
            // ---------------------------------------------------------------

            gameUIManager?.AnimateCrossHairFire();

            // Emit particles at muzzle position..
            muzzle.emit(30);
            isArmingDisabled = true;

            // We raycast only to specific physics layers..
            string[] mask = { "Enemy_HurtSpot", "NonMoving", "Wall" };
            RayCastResult? result = PhysicsAPI.Raycast(playerCamera.position, playerCamera.front, 1000f, mask);
            
            if (result != null)
            {
                GameObject ammoTrail = Instantiate(ammoTrailPrefab, muzzle.gameObject.transform.position, muzzle.gameObject.transform.rotation);
                Vector3 directionTOLookAt = result.Value.point - muzzle.gameObject.transform.position;
                directionTOLookAt.Normalize();
                muzzle.gameObject.transform.rotation = Quaternion.LookRotation(directionTOLookAt);
                ammoTrail.getScript<ammoTrails>().startPosition = muzzle.gameObject.transform.position;
                ammoTrail.getScript<ammoTrails>().endPosition = result.Value.point;

            }
            else
            {
                GameObject ammoTrail = Instantiate(ammoTrailPrefab, muzzle.gameObject.transform.position, muzzle.gameObject.transform.rotation);
                // Debug.Log("Miss");
                ammoTrail.getScript<ammoTrails>().startPosition = muzzle.gameObject.transform.position;
                ammoTrail.getScript<ammoTrails>().endPosition = muzzle.gameObject.transform.position + (playerCamera.front * 500f);
            }
        }

        if (weaponControlStates == WeaponControlStates.ThrowReady && currentlyHeldGun.gameObject.IsActive() == true)
        {
            //playerArmGun.getComponent<Animator_>().SetFrame(30);
            playerArmGun.getComponent<Animator_>().speedMultiplier = 1f;
            weaponControlStates = WeaponControlStates.AwaitWeaponReturn;

            // restart immeidately 
            weaponOffset.speedMultiplier = -1f;
        }
    }

    public void ThrowWeapon()
    {
        GameObject thrownRifle = Instantiate(thrownRiflePrefab, throwPosition.position, throwPosition.rotation);

        if (thrownRifle == null)
        {
            return;
        }

        thrownRifle.getScript<ThrowableRifle>().playerGameobject = this.gameObject;
        thrownRifle.getScript<ThrowableRifle>().mappedWeapon = currentlyHeldGun;
        weaponControlStates = WeaponControlStates.AwaitWeaponReturn;

        string[] mask = { "Enemy_HurtSpot", "NonMoving", "Wall" };

        RayCastResult? result = PhysicsAPI.Raycast(playerCamera.position, playerCamera.front, 10000f, mask);


        //Do a raycast to objects
        if (result != null)
        {
            Vector3 targetDirection = (result.Value.point - throwPosition.position);
            targetDirection.Normalize();

            thrownRifle.getScript<ThrowableRifle>().flightPath = targetDirection;
        }
        else
        {
            thrownRifle.getScript<ThrowableRifle>().flightPath = playerCamera.front;
            Vector3 endPoint = playerCamera.position + playerCamera.front * 500f;
            thrownRifle.getScript<ThrowableRifle>().SeekTarget(playerCamera.position, endPoint);
        }

        currentlyHeldGun.gameObject.SetActive(false);
        thrownRifle.getScript<ThrowableRifle>().InitWeapon();

        armTimeElapsed = 0;
    }



    public void WeaponCollected(Gun gun)
    {
        if (currentlyHeldGun == gun)
        {
            currentlyHeldGun.gameObject.SetActive(true);
            gunHolder.localPosition = gunPosition.localPosition;
            lerpVariable = 0;
            weaponControlStates = WeaponControlStates.WeaponFree;

            playerArmGun.getComponent<Animator_>().PlayAnimation(RetrieveAnimationName);
            playerArmGun.getComponent<Animator_>().SetFrame(35);

            finalGlowStrength = noAmmoGlowStrength * Mathf.Pow(1f - (float)currentlyHeldGun.CurrentAmmo / (float)currentlyHeldGun.MaxAmmo, ammoGlowScalePower);
        }
    }


    public void ResetGunPosition()
    {
        gunHolder.localPosition = gunPosition.localPosition;
    }
    public void DisableWeaponArm()
    {
        isArmingDisabled = true;
    }

    public void EnableWeaponArm()
    {
        isArmingDisabled = false;
    }

    public void DisableShooting(){ isShootingDisabled = true;  }
    public void EnableShooting() { isShootingDisabled = false; }
    public void Reset()
    {
        currentlyHeldGun.CurrentAmmo = currentlyHeldGun.MaxAmmo;
    }

    public void ArmReady()
    {
        playerArmGun.getComponent<Animator_>().speedMultiplier = 0f;
        weaponControlStates = WeaponControlStates.ThrowReady;   
    }

    public void MoveWeaponOffset()
    {
        weaponOffset.speedMultiplier = 1f;
        weaponOffset.play();    
    }
}