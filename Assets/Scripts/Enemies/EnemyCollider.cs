// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class EnemyCollider : Script
{
    [SerializableField]
    private GameObject enemyGameObject;
    private Enemy enemy;
    protected override void init()
    {
        enemy = enemyGameObject.getScript<Enemy>();
    }
    public void OnColliderShot(float damage)
    {
        enemy.TakeDamage(damage);
    }
}