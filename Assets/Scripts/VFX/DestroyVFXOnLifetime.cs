// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class DestroyVFXOnLifetime : Script
{
    // This function is invoked once before init when gameobject is active.
    protected override void init()
    {
        
        Invoke(() =>{
            if (gameObject != null)
                Destroy(gameObject);
        }, getComponent<ParticleEmitter_>().lifeTime);
    }
}