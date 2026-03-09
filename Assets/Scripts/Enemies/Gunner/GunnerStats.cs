// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class GunnerStats : EnemyStats
{
    public float shootingRange = 100f;
    public float safeVantageRange = 40f;
    public float escapeRange = 20f;
    // Movement
    public float targetDistanceFromVantagePoint = 3f;
    public float timeBetweenSteps = 0.3f;
}