// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
class DroppingBox : Script
{
    private AudioComponent_ audioComponent;

    [SerializableField]
    private GameObject? liftingBoxEmitter;
    [SerializableField]
    private ParticleEmitter_? dropBurstEmitter;
    [SerializableField]
    private Audio beginLiftSFX;
    [SerializableField]
    private Audio endDropSFX;
    protected override void init()
    {
        audioComponent = getComponent<AudioComponent_>();
    }
    public void BeginLift()
    {
        liftingBoxEmitter.SetActive(true);
        audioComponent.PlaySound(beginLiftSFX);
    }
    public void EndLift()
    {
        liftingBoxEmitter.SetActive(false);
    }
    public void EndDrop()
    {
        dropBurstEmitter.emit(100);
        Vector3 position = gameObject.transform.position;
        audioComponent.PlaySound(endDropSFX);
        
    }

}