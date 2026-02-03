// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class DroppingBox : Script
{
    [SerializableField]
    private GameObject? liftingBoxEmitter;
    [SerializableField]
    private ParticleEmitter_? dropBurstEmitter;
    [SerializableField]
    private AudioComponent_ audioComponent;
    [SerializableField]
    private Audio pickupSFX;
    [SerializableField]
    private Audio boxLandSFX;

    protected override void init()
    {
        base.init();
        audioComponent = getComponent<AudioComponent_>();
    }
    public void BeginLift()
    {
        liftingBoxEmitter.SetActive(true);
        audioComponent.PlaySound(pickupSFX);
        // AudioAPI.PlaySound(gameObject, "LaserRifle_SmallRocket_Shot1");
    }
    public void EndLift()
    {
        liftingBoxEmitter.SetActive(false);
    }
    public void EndDrop()
    {
        dropBurstEmitter.emit(100);
        Vector3 position = gameObject.transform.position;
        audioComponent.PlaySound(boxLandSFX);
        // AudioAPI.PlaySound(gameObject, "sfx_enemyChargeStep_02mono");

    }

}