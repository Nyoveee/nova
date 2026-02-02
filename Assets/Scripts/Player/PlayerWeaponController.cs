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
    public required GameObject playerCollider;
    public required Prefab thrownRiflePrefab;
    public required Prefab ammoTrailPrefab;


    public float armingTime = 0.3f;
    public float bulletSpeed;
    public float swapWeaponCooldown = 0.2f;

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

    public WeaponControlStates weaponControlStates;

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

    // This function is invoked every fixed update.
    protected override void update()
    {
        //if (recentlySwappedWeapon)
        //{
        //    if (timeElapsed < swapWeaponCooldown) {
        //        timeElapsed += Time.V_DeltaTime();
        //    }
        //    else
        //    {
        //        recentlySwappedWeapon = false;
        //        timeElapsed = 0;

        //        // after some duration, set weapon active..
        //        setWeaponActiveDelegate();
        //    }
            
        //    return;
        //}

        switch (weaponControlStates)
        {
            case WeaponControlStates.Busy:
                { //anystate you want to lock weaponcontrols
                 
                }
                break;
            case WeaponControlStates.WeaponFree:
                {
                    if (currentlyHeldGun.CurrentAmmo != 0)
                    {

                    }
                    else if (currentlyHeldGun.CurrentAmmo <= 0)
                    { 
                        weaponControlStates = WeaponControlStates.ArmingThrow;
                        
                    }
                
                }
                break;
            case WeaponControlStates.ArmingThrow:
                { 
                    armTimeElapsed += Time.V_DeltaTime();

                    float t  = armTimeElapsed / armingTime;


                    if (t >= 1)
                    {
                        weaponControlStates = WeaponControlStates.ThrowReady;
                        armTimeElapsed = armingTime;
                    }
                    else
                    {
                        gunHolder.localPosition = Vector3.Lerp(gunPosition.localPosition, throwPosition.localPosition, t);

                    }
                    

                }
                break;
            case WeaponControlStates.DisarmingFree:
                {
                    armTimeElapsed -= Time.V_DeltaTime();

                    float t = armTimeElapsed / armingTime;

                    if (t <= 0)
                    {
                        weaponControlStates = WeaponControlStates.WeaponFree;
                        armTimeElapsed = 0;
                    }
                    else 
                    {
                        gunHolder.localPosition = Vector3.Lerp(gunPosition.localPosition, throwPosition.localPosition, t);
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


    private void Arming()
    {
        if (currentlyHeldGun.CurrentAmmo != 0 && (weaponControlStates == WeaponControlStates.WeaponFree || weaponControlStates == WeaponControlStates.DisarmingFree ) )
        {

            weaponControlStates = WeaponControlStates.ArmingThrow;

        }
    
    
    }

    private void Disarming()
    {
        if (currentlyHeldGun.CurrentAmmo != 0 && (weaponControlStates == WeaponControlStates.ArmingThrow || weaponControlStates == WeaponControlStates.ThrowReady))
        {

            weaponControlStates = WeaponControlStates.DisarmingFree;

        }


    }


    private void Fire()
    {
        //if (weaponControlStates != WeaponControlStates.WeaponFree)
        //{
        //    return;

        //}


        if(weaponControlStates == WeaponControlStates.WeaponFree && currentlyHeldGun.Fire())
        {
            muzzle.emit(30);


            string[] mask = { "Enemy_HurtSpot", "NonMoving", "Wall" };
            // Raycast..
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

                ammoTrail.getScript<ammoTrails>().startPosition = muzzle.gameObject.transform.position;
                ammoTrail.getScript<ammoTrails>().endPosition = muzzle.gameObject.transform.position + (muzzle.gameObject.transform.front * 500f);

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

        //string[] layerMask = { "Enemy", "Wall", "Ground" };

       // RayCastResult? result = PhysicsAPI.Raycast(playerCamera.position, playerCamera.front,500f,layerMask);

        RayCastResult? result = PhysicsAPI.Raycast(playerCamera.position, playerCamera.front, 500f,playerCollider);


       


        //Do a raycast to objects
        if (result != null)
        {
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
            weaponControlStates = WeaponControlStates.WeaponFree;

        }




    }

    public void ResetGunPosition()
    {
        gunHolder.localPosition = gunPosition.localPosition;
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