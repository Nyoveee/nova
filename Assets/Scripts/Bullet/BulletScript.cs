// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using System;

class BulletScript : Script
{
    public float lifeTime = 2f;
    private float timeElapsed = 0f;

    // This function is first invoked when game starts.
    protected override void init()
    {}

    // This function is invoked every fixed update.
    protected override void update()
    {
        if (timeElapsed > lifeTime)
        {
            Destroy(gameObject);
        }
        else
        {
            timeElapsed += Time.V_FixedDeltaTime();
        }
    }
}