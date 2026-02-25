// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using Windows.Media.Transcoding;
using Windows.Services.Maps.LocalSearch;

class EnemyCannon : Script
{
    [SerializableField]
    private Light_ light;
    [SerializableField]
    private ParticleEmitter_ fireSmoke;
    [SerializableField]
    private ParticleEmitter_ fire;
    [SerializableField]
    private ParticleEmitter_ charge;
    [SerializableField]
    private float minTimeShootCooldown;
    [SerializableField]
    private float maxTimeShootCooldown;
    [SerializableField]
    private GameObject shootingArea;
    [SerializableField]
    private Prefab enemyPrefab;
    [SerializableField]
    private Prefab launchingVFXPrefab;
    [SerializableField]
    private float cannonTurningTime;
    [SerializableField]
    private float minArcTime;
    [SerializableField]
    private float maxArcTime;
    [SerializableField]
    private float cannonChargeTime;
    [SerializableField]
    private MeshRenderer_ cannonMeshRenderer;
    [SerializableField]
    private MeshRenderer_ cannonBarrelMeshRenderer;
    [SerializableField]
    private GameObject boat;
    [SerializableField]
    private GameObject waveManager;

    // Shooting Update
    private float arcTime;
    //private float currentShootCooldown;
    private GameObject enemyObject;

    // Shooting Arc Parameters
    private Quaternion targetRotation;
    private Quaternion startRotation;
    private Vector3 targetPosition;
    private Vector3 targetVelocity;

    // Rotation Update
    private float currentTurningTime;

    // VFX Update
    private float currentChargeTime;
    private float currentLightTime;
    private bool b_IsCharging;

    // For Wave Manager
    private float yOffset;
    private int shotsQueued = 0;

    protected override void init() 
    {
        yOffset = gameObject.transform.position.y - boat.transform.position.y;
    }

    protected override void update() {
        Vector3 pos = gameObject.transform.position;
        pos.y = boat.transform.position.y + yOffset;
        gameObject.transform.position = pos;
        // Cooldown
        //if(enemyObject == null)
        //{
        //    currentShootCooldown -= Time.V_DeltaTime();
        //    if(currentShootCooldown <= 0)
        //    {
        //        GetTargetingLocation();
        //        PrepareEnemyCannon();
        //        return;
        //    }
        //}

        // For Wave Manager
        if (shotsQueued > 0 && enemyObject == null && !b_IsCharging)
        {
            shotsQueued--;
            GetTargetingLocation();
            PrepareEnemyCannon();
        }

        // Cannon Firing
        if (b_IsCharging)
        {
            currentChargeTime -= Time.V_DeltaTime();
            if (currentChargeTime - charge.lifeTime <= 0)
                charge.enable = false;
            if (currentChargeTime <= 0)
                Fire();
        }
        // Cannon Rotation
        if (enemyObject != null)
        {
            RotateCannon();
            if (IsRotationFinished() && !b_IsCharging)
                PrepareCharge();
        }
        // Light
        currentLightTime -= Time.V_DeltaTime();
        if (currentLightTime <= 0)
            light.enable = false;
    }

    private void GetTargetingLocation() {
        Vector3 min = shootingArea.transform.position - shootingArea.transform.scale;
        Vector3 max = shootingArea.transform.position + shootingArea.transform.scale;
        Vector3 randomPoint = Random.Range(min, max);
        string[] mask = { "Floor" };
        RayCastResult? result = PhysicsAPI.Raycast(randomPoint, Vector3.Down(), 1000f, mask);
        if(result!= null)
            targetPosition = result.Value.point;
    }
 
    private void RotateCannon() {
        currentTurningTime += Time.V_DeltaTime();
        currentTurningTime = Mathf.Min(currentTurningTime, cannonTurningTime);
        gameObject.transform.localRotation = Quaternion.Slerp(startRotation, targetRotation, currentTurningTime / cannonTurningTime);
    }
    private bool IsRotationFinished() {
        return currentTurningTime == cannonTurningTime;
    }
    private void Fire() {
        enemyObject.SetActive(true);
        waveManager.getScript<CannonWaveManager>().RegisterEnemy(enemyObject);
        //currentShootCooldown = Random.Range(minTimeShootCooldown, maxTimeShootCooldown);

        // Set the velocity
        Rigidbody_ enemyRigidbody = enemyObject.getComponent<Rigidbody_>();
        enemyRigidbody.SetVelocity(targetVelocity);

        // VFX
        GameObject launchingVFX = Instantiate(launchingVFXPrefab, enemyObject);
        launchingVFX.getScript<LaunchingVFX>().SetEnemy(enemyObject.getScript<Enemy>());
        enemyObject = null;

        // Activate/Deactive VFX
        fireSmoke.emit();
        fire.emit();
        light.enable = true;
        currentLightTime = fire.lifeTime;
        b_IsCharging = false;
        cannonMeshRenderer.setMaterialBool(1, "isActive", false);
        cannonBarrelMeshRenderer.setMaterialBool(1, "isActive", false);
    }
    private void PrepareEnemyCannon()
    {
        // Setup Components
        enemyObject = Instantiate(enemyPrefab,gameObject.transform.position);
        enemyObject.transform.position -= new Vector3(0,enemyObject.transform.scale.y / 2f,0);
        enemyObject.SetActive(false);
        Rigidbody_ enemyRigidbody = enemyObject.getComponent<Rigidbody_>();

        // Physics Params
        arcTime = Random.Range(minArcTime, maxArcTime);
        float gravity = -PhysicsAPI.GetGravity() * enemyRigidbody.GetGravityFactor();
        Vector3 startPosition = gameObject.transform.position;
        Vector3 endPosition = targetPosition;
        Vector3 displacement = endPosition - startPosition;

        // Horizontal
        targetVelocity = new Vector3(displacement.x, 0, displacement.z) / arcTime;

        // Vertical
        float yVelocity = (displacement.y - 0.5f * gravity * arcTime * arcTime) / arcTime;
        targetVelocity += new Vector3(0, yVelocity, 0);
        
        // Set the rotation
        Vector3 targetDirection = targetVelocity;
        targetDirection.Normalize();
        startRotation = gameObject.transform.localRotation;
        targetRotation = Quaternion.LookRotation(targetDirection);
        
        // Set the timers
        currentTurningTime = 0;
    }
    private void PrepareCharge()
    {
        cannonMeshRenderer.setMaterialBool(1, "isActive", true);
        cannonBarrelMeshRenderer.setMaterialBool(1, "isActive", true);
        currentChargeTime = cannonChargeTime;
        charge.enable = true;
        b_IsCharging = true;
    }

    public void FireNextShot()
    {
        shotsQueued++;
    }
}