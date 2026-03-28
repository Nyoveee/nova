// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using System.Runtime.CompilerServices;
using Windows.Devices.SerialCommunication;
public abstract class Enemy : Script
{
    public enum EnemydamageType
    {
        WeaponShot,
        ThrownWeapon,
        Ultimate,
    }

    /***********************************************************
        Inspector Variables
    ***********************************************************/

    [SerializableField]
    private Prefab ichorPrefab;
    [SerializableField]
    private Prefab explodeVFXPrefab;
    [SerializableField]
    private GameObject ichorSpawnPoint;
    [SerializableField]
    protected Animator_? animator = null;
    [SerializableField]
    protected SkinnedMeshRenderer_? renderer = null;
    [SerializableField]
    protected NavMeshAgent_? navMeshAgent = null;
    [SerializableField]
    public List<GameObject> enemyColliders;
    // 1f = 100% 
    [SerializableField]
    public float spotCallSFXChance = 0.5f;
    [SerializableField]
    protected float movementStaggerTime = 0.1f;
    [SerializableField]
    protected Rigidbody_ physicsRigidbody;
    [SerializableField]
    protected Rigidbody_ navMeshRigidBody; //LEGACY CODE, navmesh rigidbody should be turned into a normal hurt box collider in the future we should not need a navmesh rigidbody
    [SerializableField]
    private float maxEmissiveValue;
    [SerializableField]
    private int deathFlickerIndex;
    [SerializableField]
    private int deathFlickerAmount;
    [SerializableField]
    private float deathFlickerSpeed;
    [SerializableField]
    private float deathFlickerDelay;
    [SerializableField]
    private float deathFlickerMinSpeedOffset;
    [SerializableField]
    private float deathFlickerMaxSpeedOffset;
    [SerializableField]
    private float ichorSpawnPositionVarianceMax = 1.5f;
    [SerializableField]
    private float ichorSpawnPositionVarianceMin = 0f;

    /***********************************************************
        Local Variables
    ***********************************************************/
    protected GameObject? player = null;
    protected GameObject? playerHead = null;
    protected EnemyStats? enemyStats = null;
    private bool wasRecentlyDamaged = false;

    private Vector3? navigationTargetPosition = null;
    protected float accumulatedDamageInstance = 0f;
    // Jump
    private float currentJumpDuration = 0f;
    private NavMeshOfflinkData offlinkData;
    private float verticalMaxJumpHeight;
    // Death
    private float currentDeathEmissiveTime;
    private float currentFlickerTime;
    private float currentFlickerVariance;
    private bool b_isFlickering = false;
    /***********************************************************
        Enemy Types must inherited from this
    ***********************************************************/
    public abstract void TakeDamage(float damage, EnemydamageType type, string colliderTag);
    public abstract bool IsEngagedInBattle();
    // when invoked, this function puts the enemy into idle state without chasing capability for `seconds` duration.
    public abstract void SetSpawningDuration(float seconds);
    /***********************************************************
        Enemy Types cam inherit from this
    ***********************************************************/
    public virtual void StaggerMovement()
    {
        if (gameObject == null || navigationTargetPosition == null)
            return;
        NavigationAPI.stopAgent(gameObject);
        animator.speedMultiplier = 0;
        Invoke(() =>
        {
            if (gameObject != null)
            {
                MoveToNavMeshPosition(navigationTargetPosition.Value);
                animator.speedMultiplier = 1;
            }
        }, movementStaggerTime);
    }
    /***********************************************************
       Public Functions
    ***********************************************************/
    public bool IsDead() => (enemyStats.health <= 0);
    public void ActivateDeathFlicker()
    {
        Invoke(() =>
        {
            b_isFlickering = true;
        }, deathFlickerDelay);
    }
    public bool IsExecutable() => (enemyStats.health <= enemyStats.enemyExecuteThreshold && enemyStats.health > 0);
    public bool WasRecentlyDamaged() => wasRecentlyDamaged;
    public void UpdateExecutableMaterialState() { renderer.setMaterialBool(1, "isActive", IsExecutable()); }
   
    public void Explode()
    {
        for (int i = 0; i < enemyStats.ichorExplodeSpawnAmount; ++i)
        {
            Vector3 direction = new Vector3(0, Random.Range(-1f, 1f), 0);
            direction.Normalize();
            float spawnDistance = Random.Range(ichorSpawnPositionVarianceMin, ichorSpawnPositionVarianceMax);
            Instantiate(ichorPrefab, ichorSpawnPoint.transform.position + direction * spawnDistance);
        }
        // Explode VFX
        GameObject explodeVFX = Instantiate(explodeVFXPrefab, ichorSpawnPoint.transform.position);
        foreach (GameObject emitter in explodeVFX.GetChildren())
        {
            ParticleEmitter_? particleEmitter_ = emitter.getComponent<ParticleEmitter_>();
            if (particleEmitter_ != null)
                particleEmitter_.emit();
        }
    }
    public bool IsTouchingGround()
    {
        string[] mask = { "Floor" };
        var result = PhysicsAPI.Raycast(gameObject.transform.position, Vector3.Down(), enemyStats.groundDetectionRayCast, mask);
        if(result != null){
            gameObject.transform.position = result.Value.point;
            return true;
        }
        result = PhysicsAPI.Raycast(gameObject.transform.position, Vector3.Up(), enemyStats.groundDetectionRayCast, mask);
        if (result != null)
        {
            gameObject.transform.position = result.Value.point;
            return true;
        }
        return false;
    }
    public bool IsTargetNavigationPositionReached()
    {
        if (navigationTargetPosition == null)
            return false;
        string[] mask = { "Floor" };
        var result = PhysicsAPI.Raycast(gameObject.transform.position, Vector3.Down(), 1000, mask);
        if (result == null)
            return false;
        Vector3 positionOnGround = result.Value.point;
        return (navigationTargetPosition.Value - positionOnGround).Length() <= 1f;
    }
    //Yo btw .enable/disable does not actually work???, so just set object inactive better

    public void ActivateRigidbody()
    {
        physicsRigidbody.SetVelocity(Vector3.Zero());
        navMeshAgent.enable = false;
        physicsRigidbody.enable = true;
        //navMeshRigidBody.enable = false;
    }
    public void ActivateNavMeshAgent()
    {
        navMeshAgent.enable = true;
        physicsRigidbody.enable = false;
        //navMeshRigidBody.enable = true;
    }
    public void DisablePhysicalInteraction()
    {   
        foreach(var collider in enemyColliders)
        {
            collider.getComponent<Rigidbody_>().enable = false;
            //collider.SetActive(false);
        }

        navMeshAgent.enable = false;
    }
    /***********************************************************
        Shared Functions
    ***********************************************************/
    protected Vector3 GetTargetJumpPosition()
    {
        return offlinkData.endNode;
    }
    protected bool IsCurrentlyJumping()
    {
        return offlinkData.valid && navMeshAgent.isOnOffMeshLinks();
    }
    protected bool IsJumpFinished()
    {
        float t = currentJumpDuration / enemyStats.jumpDuration;
        Vector3 horizontalPos = Vector3.Lerp(offlinkData.startNode, offlinkData.endNode, t);
        float yOffset = verticalMaxJumpHeight * 4f * (t - t * t);
        gameObject.transform.position = horizontalPos + Vector3.Up() * yOffset;
        currentJumpDuration += Time.V_DeltaTime();
        return currentJumpDuration >= enemyStats.jumpDuration;
    }

    protected void LookAt(GameObject @object)
    {
        if (@object == null)
            return;
        Vector3 direction = @object.transform.position - renderer.gameObject.transform.position;
        direction.y = 0;
        direction.Normalize();

        gameObject.transform.setFront(direction);
    }
    protected void LookAt(Vector3 position)
    {
        Vector3 direction = position - renderer.gameObject.transform.position;
        direction.y = 0;
        direction.Normalize();

        gameObject.transform.setFront(direction);
    }
    protected float GetDistanceFromPlayer()
    {
        return player != null ? Vector3.Distance(player.transform.position, gameObject.transform.position) : 0f;
    }
    protected bool HasLineOfSight(GameObject from)
    {
        string[] layerMask = { "Wall","Floor" };
        float distance = Vector3.Distance(from.transform.position, playerHead.transform.position);
        return PhysicsAPI.Linecast(from.transform.position, playerHead.transform.position, layerMask) == null;
    }

    protected void SpawnIchorFrame()
    {
        int currentSpawnAmount = (int)(accumulatedDamageInstance / enemyStats.ichorPerDamage);

        for (int i = 0; i < currentSpawnAmount; ++i)
        {
            //Vector3 direction = new Vector3(0, Random.Range(-1f, 1f), 0);
            //direction.Normalize();
            float spawnDistance = Random.Range(ichorSpawnPositionVarianceMin, ichorSpawnPositionVarianceMax);
            GameObject ichor = Instantiate(ichorPrefab);
            ichor.transform.position = ichorSpawnPoint.transform.position + ichor.getScript<Ichor>().GetDirection() * spawnDistance;
        }

    }

    protected void MoveToNavMeshPosition(Vector3 position)
    {
        string[] layerMask = { "Floor" };
        RayCastResult? result = PhysicsAPI.Raycast(position, Vector3.Down(), 1000f,layerMask);
        if (result != null)
        {
            navigationTargetPosition = result.Value.point;
            NavigationAPI.setDestination(gameObject, result.Value.point);
        }       
    }
    protected void StopNavMeshMovement()
    {
        string[] layerMask = { "Floor" };
        RayCastResult? result = PhysicsAPI.Raycast(gameObject.transform.position, Vector3.Down(), 1000f, layerMask);
        if (result != null)
            navigationTargetPosition = result.Value.point;
        NavigationAPI.stopAgent(gameObject);
    }
    protected void TriggerRecentlyDamageCountdown()
    {
        wasRecentlyDamaged = true;
        Invoke(() =>
        {
            wasRecentlyDamaged = false;
        }, enemyStats.hurtDuration);
    }
    protected bool IsOnNavMeshOfflink() {
        if (!navMeshAgent.isOnOffMeshLinks())
            return false;
        offlinkData = navMeshAgent.getOffLinkData();
        if (offlinkData.valid){
            verticalMaxJumpHeight = MathF.Abs(offlinkData.endNode.y - offlinkData.startNode.y);
            verticalMaxJumpHeight = 0.25f + 0.5f * verticalMaxJumpHeight; // 0.25f adds small curve
            currentJumpDuration = 0f;
        }
        
        return offlinkData.valid;
    }
    private void DeathFlicker()
    {
        currentDeathEmissiveTime += Time.V_DeltaTime();
        float currentDeathFlickerSpeed = deathFlickerSpeed + currentFlickerVariance;
        if(currentDeathEmissiveTime > deathFlickerSpeed){
            currentFlickerVariance = Random.Range(deathFlickerMinSpeedOffset, deathFlickerMaxSpeedOffset);
            currentDeathEmissiveTime = 0;
            --deathFlickerAmount;
        }
        else if (currentDeathEmissiveTime > currentDeathFlickerSpeed / 2f) {
            renderer.setMaterialFloat(deathFlickerIndex, "emissiveStrength", maxEmissiveValue);
        }
        else {
            renderer.setMaterialFloat(deathFlickerIndex, "emissiveStrength", 0);
        }
    }
    /***********************************************************
       Script Functions
    ***********************************************************/
    protected override void init()
    {
        enemyStats = getScript<EnemyStats>();
        player = GameObject.FindWithTag("Player");
        playerHead = GameObject.FindWithTag("PlayerHead");
        navMeshAgent.setAutomateNavMeshOfflinksState(false);
        renderer.setMaterialFloat(deathFlickerIndex, "emissiveStrength", maxEmissiveValue);
        currentFlickerVariance = Random.Range(deathFlickerMinSpeedOffset, deathFlickerMaxSpeedOffset);
    }
    protected override void update()
    {
        if (b_isFlickering)
        {
            if (deathFlickerAmount > 0)
                DeathFlicker();
            else
                renderer.setMaterialFloat(deathFlickerIndex, "emissiveStrength", 0);
        }
    }

    protected override void onCollisionEnter(GameObject other)
    {
    }


}