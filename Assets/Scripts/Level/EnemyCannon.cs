// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using Windows.Services.Maps.LocalSearch;

class EnemyCannon : Script
{
    [SerializableField]
    private float minTimeShootCooldown;
    [SerializableField]
    private float maxTimeShootCooldown;
    [SerializableField]
    private GameObject shootingArea;
    [SerializableField]
    private Prefab enemyPrefab;
    [SerializableField]
    private float cannonTurningTime;
    [SerializableField]
    private float arcTime;

    // Shooting Update
    private float currentShootCooldown;
    private GameObject enemyObject;

    // Shooting Arc Parameters
    private Quaternion targetRotation;
    private Quaternion startRotation;
    private Vector3 targetPosition;
    private Vector3 targetVelocity;

    // Rotation Update
    private float currentTurningTime;

    protected override void update() {
        if(enemyObject == null)
        {
            currentShootCooldown -= Time.V_DeltaTime();
            if(currentShootCooldown <= 0)
            {
                GetTargetingLocation();
                PrepareEnemy();
                return;
            }
        }
        if(enemyObject != null)
        {
            RotateCannon();
            if (IsRotationFinished())
                ShootEnemy();
        }
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
    private void ShootEnemy() {
        enemyObject.SetActive(true);
        currentShootCooldown = Random.Range(minTimeShootCooldown, maxTimeShootCooldown);

        // Set the velocity
        Rigidbody_ enemyRigidbody = enemyObject.getComponent<Rigidbody_>();
        enemyRigidbody.SetVelocity(targetVelocity);
        
        enemyObject = null;
    }
    private void PrepareEnemy()
    {
        // Setup Components
        enemyObject = Instantiate(enemyPrefab,gameObject.transform.position);
        enemyObject.SetActive(false);
        Rigidbody_ enemyRigidbody = enemyObject.getComponent<Rigidbody_>();

        // Physics Params
        float gravity = -PhysicsAPI.GetGravity() * enemyRigidbody.GetGravityFactor();
        Vector3 startPosition = gameObject.transform.position;
        Vector3 endPosition = targetPosition;
        Vector3 displacement = endPosition - startPosition;

        // I'm not sure why the movement update is not accurate
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

}