// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using static System.Net.Mime.MediaTypeNames;

class UltimateProjectile : Script
{
    public Prefab ultimateExplosion;
    public float lifetime = 3f;

    // This function is first invoked when game starts.
    protected override void init()
    {
        Debug.Log("walls");
    }

    // This function is invoked every update.
    protected override void update()
    {}

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

    protected override void onCollisionEnter(GameObject other)
    {
        if (other.tag == "Wall")
        {
            if (gameObject != null)
            {
                Instantiate(ultimateExplosion, gameObject.transform.position);
                Destroy(gameObject);
            }
        }
    }

}