// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class Ichor : Script
{
    /***********************************************************
        Inspector Variables
    ***********************************************************/
    [SerializableField]
    private float ichorLifeTime = 10f;
    [SerializableField]
    private float initialForce = 2000f;
    /***********************************************************
        Local Variables
    ***********************************************************/
    private Rigidbody_ rigidbody;
    protected override void init()
    {
        rigidbody = getComponent<Rigidbody_>();
        Vector3 direction = new Vector3(Random.Range(-1f,1f),Random.Range(0f,1f),Random.Range(-1f,1f));
        direction.Normalize();
        rigidbody.AddForce(direction * initialForce);
        Invoke(() =>
        {
            Destroy(gameObject);
        }, ichorLifeTime);
    }
}