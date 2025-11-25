// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class ChargerStats : EnemyStats
{
    public float damage = 20f;
    // Movement
    public float timeBetweenChargeSteps = 0.26f;
    // Combat
    public int chargeDamage;
    public float chargeCooldown;
    public float maxChargeTime;
    public float speedMultiplier;
    // Detection
    public float chasingRange;
    public float chargingRange;
    public float attackRange;
}