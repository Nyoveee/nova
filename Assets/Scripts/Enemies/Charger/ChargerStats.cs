// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class ChargerStats : EnemyStats
{
    public float damage = 20f;
    // Movement
    public float timeBetweenChargeSteps = 0.26f;
    // Charging
    public float chargingRange;
    public int chargeDamage;
    public float chargeCooldown;
    public float maxChargeTime;
    public float speedMultiplier;
    // Stomping
    public int stompDamage = 20;
    public float stompCooldown = 20f;
    public float stompHitboxDuration = 0.1f;
    // Detection
    public float chasingRange;
    public float attackRange;
}