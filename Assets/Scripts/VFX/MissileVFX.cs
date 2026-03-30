// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class MissileVFX : Script
{

    //Duration
    [SerializableField]
    private float duration;

    private float timeElasped = 0;

    [SerializableField]
    private ParticleEmitter_ light;
    
    [SerializableField]
    private ParticleEmitter_ explosion;
    
    [SerializableField]
    private ParticleEmitter_ sparks1;
    
    [SerializableField]
    private ParticleEmitter_ sparks2;

    // This function is invoked once before init when gameobject is active.

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        //light.emit();
        //explosion.emit();
        //sparks1.emit();
        //sparks2.emit();



    }

    // This function is invoked every update.
    protected override void update()
    {
        if (timeElasped == 0)
        {
            light.emit();
            explosion.emit();
            sparks1.emit();
            sparks2.emit();

        }

        timeElasped += Time.V_DeltaTime();

        if (timeElasped >= duration)
        {
            Destroy(this.gameObject);
        
        
        }


    
    }


}