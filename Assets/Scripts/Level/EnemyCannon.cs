// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using Windows.Media.Transcoding;
using Windows.Services.Maps.LocalSearch;

class EnemyCannon : Script
{
    public Prefab enemyPrefab;
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
    private List<GameObject> shootingAreas;
    [SerializableField]
    private Prefab launchingVFXPrefab;
    [SerializableField]
    private float minArcTime;
    [SerializableField]
    private float maxArcTime;
    [SerializableField]
    private float minTurningTime;
    [SerializableField]
    private float maxTurningTime;
    [SerializableField]
    private float cannonChargeTime;
    [SerializableField]
    private MeshRenderer_ cannonBarrelMeshRenderer;
    [SerializableField]
    private GameObject firingPoint;
    [SerializableField]
    private Transform_ playerbody;
    [SerializableField]
    private GameObject boat;
    [SerializableField]
    private GameObject waveManager;

    // Shooting Update
    private float arcTime;
    private float currentShootCooldown;
    private GameObject enemyObject;

    // Shooting Arc Parameters
    private Quaternion targetRotation;
    private Quaternion targetCannonBarrelRotation;

    private Quaternion startRotation;
    private Quaternion startCannonBarrelRotation;

    private Vector3 targetPosition;
    private Vector3 targetVelocity;

    // Rotation Update
    private float cannonTurningTime;
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
        if (!waveManager.getScript<CannonWaveManager>().IsWaveActive())
        {
            fireSmoke.enable = false;
            fire.enable = false;
            charge.enable = false;
            return;
        }
            
        Vector3 pos = gameObject.transform.position;
        pos.y = boat.transform.position.y + yOffset;
        gameObject.transform.position = pos;

        // Cooldown
        if (enemyObject == null && !b_IsCharging)
        {
            currentShootCooldown -= Time.V_DeltaTime();
            if (currentShootCooldown <= 0)
            {
                GetTargetingLocation();
                EstimateCannonRotation();
                return;
            }
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
        List<GameObject> validTargetAreas = GetValidTargetingAreas();
        if (validTargetAreas.Count == 0)
        {
            Debug.LogError("No Valid Targeting Areas found");
            targetPosition = Vector3.Zero();
            return;
        }
        GameObject shootingArea = validTargetAreas[Random.Range(0, validTargetAreas.Count)];
        Vector3 min = shootingArea.transform.position - shootingArea.transform.scale;
        Vector3 max = shootingArea.transform.position + shootingArea.transform.scale;
        Vector3 randomPoint = Random.Range(min, max);
        string[] mask = { "Floor" };
        RayCastResult? result = PhysicsAPI.Raycast(randomPoint, Vector3.Down(), 1000f, mask);
        if(result!= null)
            targetPosition = result.Value.point;
    }
    private List<GameObject> GetValidTargetingAreas()
    {
        List<GameObject> validTargetAreas = new List<GameObject>();
        foreach (GameObject targetingArea in shootingAreas)
        {
            Vector3 min = targetingArea.transform.position - targetingArea.transform.scale;
            Vector3 max = targetingArea.transform.position + targetingArea.transform.scale;
            min.y = max.y = 0;
            if (playerbody.position.x >= min.x && playerbody.position.x <= max.x
                && playerbody.position.z >= min.z && playerbody.position.z <= max.z)
                continue;
            validTargetAreas.Add(targetingArea);
        }
        return validTargetAreas;
    }
    private void RotateCannon() {
        currentTurningTime += Time.V_DeltaTime();
        currentTurningTime = Mathf.Min(currentTurningTime, cannonTurningTime);

        cannonBarrelMeshRenderer.gameObject.transform.rotation = Quaternion.Slerp(startCannonBarrelRotation, targetCannonBarrelRotation, currentTurningTime / cannonTurningTime);
        gameObject.transform.rotation = Quaternion.Slerp(startRotation, targetRotation, currentTurningTime / cannonTurningTime);
    }

    private bool IsRotationFinished() {
        return currentTurningTime == cannonTurningTime;
    }
    private void Fire() {
        currentShootCooldown = Random.Range(minTimeShootCooldown, maxTimeShootCooldown);
        enemyObject.SetActive(true);
        enemyObject.transform.position = firingPoint.transform.position;

        // Set the velocity
        GetTrajectory(firingPoint.transform.position);
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
        cannonBarrelMeshRenderer.setMaterialBool(1, "isActive", false);
    }
    // Rotation may look slightly different close up but it's good enough for now
    private void EstimateCannonRotation()
    {
        // Setup Components
        enemyObject = Instantiate(enemyPrefab);
        enemyObject.transform.position -= new Vector3(0,enemyObject.transform.scale.y / 2f,0);
        enemyObject.SetActive(false);

        GetTrajectory(gameObject.transform.position);
        
        // Set the rotation
        Vector3 targetDirection = targetVelocity;
        targetDirection.Normalize();

        startRotation = gameObject.transform.rotation;
        startCannonBarrelRotation = cannonBarrelMeshRenderer.gameObject.transform.rotation;

        targetRotation = Quaternion.LookRotation(targetDirection);
        Vector3 eulerAngle = Rotation.ToEuler(targetRotation); 
        eulerAngle = new Vector3(180f * Mathf.Deg2Rad, eulerAngle.y, eulerAngle.z);
        targetRotation = Rotation.ToQuaternion(eulerAngle);

        targetCannonBarrelRotation = Quaternion.LookRotation(-targetDirection);

        // Set the timers
        cannonTurningTime = Random.Range(minTurningTime, maxTurningTime);
        currentTurningTime = 0;
    }
   
    private void GetTrajectory(Vector3 origin)
    {
        // Physics Params
        Rigidbody_ enemyRigidbody = enemyObject.getComponent<Rigidbody_>();
        arcTime = Random.Range(minArcTime, maxArcTime);
        float gravity = -PhysicsAPI.GetGravity() * enemyRigidbody.GetGravityFactor();
        Vector3 startPosition = origin;
        Vector3 endPosition = targetPosition;
        Vector3 displacement = endPosition - startPosition;

        // Horizontal
        targetVelocity = new Vector3(displacement.x, 0, displacement.z) / arcTime;

        // Vertical
        float yVelocity = (displacement.y - 0.5f * gravity * arcTime * arcTime) / arcTime;
        targetVelocity += new Vector3(0, yVelocity, 0);
    }
    private void PrepareCharge()
    {
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