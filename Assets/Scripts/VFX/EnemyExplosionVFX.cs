// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class EnemyExplosionVFX : Script
{
    [SerializableField]
    private float explosionTime;
    // This function is invoked once when gameobject is active.
    protected override void init()
    {

        Invoke(() =>
        {
            Destroy(gameObject);
        },explosionTime);
    }
}