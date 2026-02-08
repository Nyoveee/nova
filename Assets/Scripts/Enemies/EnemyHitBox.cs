// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.


// We should phase out hitbox prefabs if possible
class EnemyHitBox : Script
{
    private bool b_HitPlayerThisAttack = false;
    private float damage;
    public void OnPlayerHit() { b_HitPlayerThisAttack = true; }
    public bool HasHitPlayerThisAttack() { return b_HitPlayerThisAttack; }
    public void SetDamage(float damage) { this.damage = damage; }
    public void ResetValues() { this.damage = 0; b_HitPlayerThisAttack = false; }
    public float GetDamage() {  return damage; }


    protected override void onCollisionEnter(GameObject other)
    {
        Debug.Log("EnemyHit box collided with:" + other.name);
        Debug.Log("Damage: " + damage);

    }
}