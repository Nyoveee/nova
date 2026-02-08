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
    public required Prefab thrownRiflePrefab;
    public required Prefab ammoTrailPrefab;


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
    private Gun currentlyHeldGun;
    private float timeElapsed;
    private float armTimeElapsed = 0f;
    private bool isArmingDisabled = false; //required by animation controller disable animations while other animation are playing
    private bool isArmingRequest = false;

    public WeaponControlStates weaponControlStates;

    private float glowChangeDuration = 0f;
    private float glowTimeElapsed = 0f;
    private float initialGlowStrength;
    private float finalGlowStrength;

    const int SNIPER_MATERIAL_MATERIAL_INDEX = 0;
    const int SNIPER_BARREL_MATERIAL_INDEX = 1;

    private float lerpVariable = 0;

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

    protected override void init()
    {
        gameUIManager = GameObject.FindWithTag("Game UI Manager")?.getScript<GameUIManager>();
        MapKey(Key.MouseLeft, Fire);
        MapKey(Key.MouseRight, Arming,Disarming);

        //ScrollCallback(SwapWeaponHandler);
        weaponControlStates = WeaponControlStates.WeaponFree;
        currentlyHeldGun = sniper;
        
        audioComponent = getComponent<AudioComponent_>();
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
                        
                        weaponControlStates = WeaponControlStates.ArmingThrow;

                    }

                    //player is trying to arm while weapon is busy with animation, now animation is over play arming
                    if (isArmingRequest == true && isArmingDisabled == false)
                    { 
                        weaponControlStates = WeaponControlStates.ArmingThrow;
                        isArmingRequest = false;
                    }

                }
                break;
            case WeaponControlStates.ArmingThrow:
                {
                    armTimeElapsed += Time.V_DeltaTime();

                    //float t = armTimeElapsed / armingTime;
                    lerpVariable = armTimeElapsed / armingTime;

                    if (lerpVariable >= 1)
                    {
                        weaponControlStates = WeaponControlStates.ThrowReady;
                        armTimeElapsed = armingTime;
                        lerpVariable = 1;
                    }
                    else
                    {
                       // gunHolder.localPosition = Vector3.Lerp(gunPosition.localPosition, throwPosition.localPosition, t);

                    }


                }
                break;
            case WeaponControlStates.DisarmingFree:
                {
                    armTimeElapsed -= Time.V_DeltaTime();

                    lerpVariable = armTimeElapsed / armingTime;

                    if (lerpVariable <= 0)
                    {
                        weaponControlStates = WeaponControlStates.WeaponFree;
                        armTimeElapsed = 0;
                        lerpVariable = 0;
                    }
                    else
                    {
                      //  gunHolder.localPosition = Vector3.Lerp(gunPosition.localPosition, throwPosition.localPosition, t);
                    }
                }
                break;
            case WeaponControlStates.ThrowReady:
                { }


                break;
            case WeaponControlStates.AwaitWeaponReturn:
                {

                }
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
        if(glowTimeElapsed < glowChangeDuration)
        {
            float interval = glowTimeElapsed / glowChangeDuration;
            float glowIntensity = Mathf.Interpolate(initialGlowStrength, finalGlowStrength, interval, 1);

            sniperMesh.setMaterialFloat(SNIPER_BARREL_MATERIAL_INDEX, "glowStrength", glowIntensity);
            sniperMesh.setMaterialFloat(SNIPER_MATERIAL_MATERIAL_INDEX, "glowStrength", glowIntensity);
        }

        glowTimeElapsed += Time.V_DeltaTime();
    }

    private void Arming()
    {
        if (currentlyHeldGun.CurrentAmmo != 0 && (weaponControlStates == WeaponControlStates.WeaponFree || weaponControlStates == WeaponControlStates.DisarmingFree ) && isArmingDisabled == false)
        {
            weaponControlStates = WeaponControlStates.ArmingThrow;

        }


        if (isArmingDisabled)
        {
            isArmingRequest = true;
        }
    }

    private void Disarming()
    {
        if (currentlyHeldGun.CurrentAmmo != 0 && (weaponControlStates == WeaponControlStates.ArmingThrow || weaponControlStates == WeaponControlStates.ThrowReady))
        {

            weaponControlStates = WeaponControlStates.DisarmingFree;
            isArmingRequest = false;

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
    }

    private void Fire()
    {
        if(weaponControlStates == WeaponControlStates.WeaponFree && currentlyHeldGun.Fire())
        {
            // ---------------------------------------------------------------
            // The moment this gun fires, the brightness of the glow spikes.
            sniperMesh.setMaterialFloat(SNIPER_BARREL_MATERIAL_INDEX, "glowStrength", peakGlowStrength);
            sniperMesh.setMaterialFloat(SNIPER_MATERIAL_MATERIAL_INDEX, "glowStrength", peakGlowStrength);

            AnimateGunGlow();
            // ---------------------------------------------------------------

            // Emit particles at muzzle position..
            muzzle.emit(30);
            isArmingDisabled = true;


            // We raycast only to specific physics layers..
            string[] mask = { "Enemy_HurtSpot", "NonMoving", "Wall" };
            RayCastResult? result = PhysicsAPI.Raycast(playerCamera.position, playerCamera.front, 500f, mask);
            
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
                Debug.Log("Miss");
                ammoTrail.getScript<ammoTrails>().startPosition = muzzle.gameObject.transform.position;
                ammoTrail.getScript<ammoTrails>().endPosition = muzzle.gameObject.transform.position + (muzzle.gameObject.transform.right * 500f);

            }



        }

        if (weaponControlStates == WeaponControlStates.ThrowReady && currentlyHeldGun.gameObject.IsActive() == true)
        {
            ThrowWeapon();
            
        }
    }

    private void ThrowWeapon()
    {

        GameObject thrownRifle = Instantiate(thrownRiflePrefab, throwPosition.position, throwPosition.rotation);
          
        if(thrownRifle == null)
        {
            return;
        }

        thrownRifle.getScript<ThrowableRifle>().playerGameobject  = this.gameObject;
        thrownRifle.getScript<ThrowableRifle>().mappedWeapon = currentlyHeldGun;
        weaponControlStates = WeaponControlStates.AwaitWeaponReturn;

        string[] mask = { "Enemy_HurtSpot", "NonMoving", "Wall" };

        // RayCastResult? result = PhysicsAPI.Raycast(playerCamera.position, playerCamera.front,500f,layerMask);

        RayCastResult? result = PhysicsAPI.Raycast(playerCamera.position, playerCamera.front, 500f, mask);


       


        //Do a raycast to objects
        if (result != null)
        {
            Debug.Log("Hit: " + result.Value.point.ToString());
            Vector3 targetDirection = (result.Value.point - throwPosition.position);
            targetDirection.Normalize();

            thrownRifle.getScript<ThrowableRifle>().flightPath = targetDirection;

            thrownRifle.getScript<ThrowableRifle>().SeekTarget(playerCamera.position, result.Value.point);


        }
        else
        {
            thrownRifle.getScript<ThrowableRifle>().flightPath = playerCamera.front;

            Vector3 endPoint = (playerCamera.front - playerCamera.position) * 500f;

            endPoint += playerCamera.position;


            thrownRifle.getScript<ThrowableRifle>().SeekTarget(throwPosition.position,endPoint);
        }

        currentlyHeldGun.gameObject.SetActive(false);
        thrownRifle.getScript<ThrowableRifle>().InitWeapon();

        weaponControlStates = WeaponControlStates.AwaitWeaponReturn;
        armTimeElapsed = 0;
    }



    public void WeaponCollected(Gun gun)
    {
        if (currentlyHeldGun == gun)
        {
            currentlyHeldGun.gameObject.SetActive(true);
            gunHolder.localPosition =  gunPosition.localPosition;
            lerpVariable = 0;
            weaponControlStates = WeaponControlStates.WeaponFree;

            AnimateGunGlow();
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

    public void Reset()
    {
        currentlyHeldGun.CurrentAmmo = currentlyHeldGun.MaxAmmo;
    }

    //public GameObject SeekTarget(Vector3 origin, Vector3 end, float seekDistance)
    //{
    //    GameObject[] candidateEnemies = GameObject.FindGameObjectsWithTag("Enemy_WeakSpot");

    //    Vector3 rayCast = end - origin;

    //    GameObject candidateTarget = null;

    //    float smallestDistance = float.PositiveInfinity;

    //    foreach (var enemy in candidateEnemies)
    //    {
    //        Vector3 otherVector = enemy.transform.position - origin;

    //        Vector3 pointOnLine = Vector3.Proj(otherVector, rayCast);

    //        if (Vector3.Distance(pointOnLine + origin, enemy.transform.position) > seekDistance)
    //        {
    //            continue;
    //        }

    //        float t = Vector3.Dot(pointOnLine, rayCast);


    //        //is on line segment?
    //        if (t > 0 && pointOnLine.Length() < rayCast.Length())
    //        {
    //            float currentDistance = Vector3.Distance(pointOnLine + origin, enemy.transform.position);

    //            if (currentDistance < smallestDistance)
    //            {

    //                smallestDistance = currentDistance;
    //                candidateTarget = enemy;
    //            }

    //        }




    //    }
    //    if (candidateTarget != null)
    //    {
    //        return candidateTarget;
    //    }
    //    return null;

    //}




    private void SwapWeaponHandler(float scrollDelta)
    {
        //if (recentlySwappedWeapon)
        //{
        //    return;
        //}

        //recentlySwappedWeapon = true;
        // since we only have 2 guns, the scroll delta doesnt really matter haha..
        // in the future if we have more we need to account for it..

        //if (currentlyHeldGun is Sniper)
        //{
        //    SwapWeapon(sniper, shotgun);
        //}
        //else
        //{
        //    SwapWeapon(shotgun, sniper);
        //}
    }

    //private void SwapWeapon(Gun from, Gun to)
    //{
    //    currentlyHeldGun = to;
    //    from.gameObject.SetActive(false);

    //    // good old javascript syntax.
    //    setWeaponActiveDelegate = () =>
    //    {
    //        to.gameObject.SetActive(true);
    //        AudioAPI.PlaySound(gameObject, "Holster SFX");
    //        if(gameUIManager!= null)
    //            gameUIManager.SwapWeaponUI(from, to);
    //    };
    //}
}