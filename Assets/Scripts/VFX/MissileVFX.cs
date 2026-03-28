// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class MissileVFX : Script
{

    //Duration
    [SerializableField]
    private float duration;

    private float timeElasped = 0;

    // This function is invoked every update.
    protected override void update()
    {
        timeElasped += Time.V_DeltaTime();

        if (timeElasped >= duration)
        {
            Destroy(this.gameObject);
        }
    }


}