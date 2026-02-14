// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class EnemyCannon : Script
{
    [SerializableField]
    private float minTimeShootCooldown;
    [SerializableField]
    private float maxTimeShootCooldown;
    [SerializableField]
    private GameObject shootingPoint;
    [SerializableField]
    private GameObject shootingTarget;
    [SerializableField]
    private Prefab enemyPrefab;
    [SerializableField]
    private float cannonTurningTime;

    // Shooting Update
    private float currentShootCooldown;

    // Shooting Arc Parameters
    private Quaternion targetRotation;
    private Quaternion startRotation;
    private float targetForce;

    // Rotation Update
    private float currentTurningTime;


    protected override void init() {}
    protected override void update() { }
    private Vector3 GetTargettingLocation() { return Vector3.Zero(); }
    private void CalculateShootingParameters(){ } 
    private void RotateCannon() { }
    private bool IsRotationFinished() { return false; }
    private void ShootEnemy() {  }

}