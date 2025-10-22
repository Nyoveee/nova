// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class EmitParticles : Script
{
    public int emitAmount = 0;
    private ParticleEmitter_? emitter;
    // This function is first invoked when game starts.
    protected override void init()
    {
        emitter = getComponent<ParticleEmitter_>();
        Input.MapKey(Key.I, OnInputPressedI, null);
    }
    private void OnInputPressedI()
    {
        if(emitter != null)
            emitter.emit(emitAmount);
    }

}