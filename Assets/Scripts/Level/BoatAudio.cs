// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
class BoatAudio : Script
{
    [SerializableField]
    private Audio waterLoopSFX;
    [SerializableField]
    private Audio waterEndLoopSFX;
    [SerializableField]
    private Audio boatStartSFX;
    [SerializableField]
    private Audio boatLoopSFX;
    [SerializableField]
    private Audio boatEndSFX;
    [SerializableField]
    private float beginLoopTime;

    private AudioComponent_ audioComponent;

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        audioComponent = getComponent<AudioComponent_>();
        audioComponent.PlaySound(waterLoopSFX);
    }

    public void BeginBoatAudio()
    {
        audioComponent.PlaySound(boatStartSFX);
        Invoke(() =>
        {
            audioComponent.StopSound(boatStartSFX);
            audioComponent.PlaySound(boatLoopSFX);
        }, beginLoopTime);
    }
    public void EndBoatAudio()
    {

    }

}