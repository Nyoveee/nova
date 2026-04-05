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
    private AudioComponent_ waterLoopAudioComponent;
    [SerializableField]
    private AudioComponent_ waterEndLoopAudioComponent;
    [SerializableField]
    private AudioComponent_ boatStartAudioComponent;
    [SerializableField]
    private AudioComponent_ boatLoopAudioComponent;
    [SerializableField]
    private AudioComponent_ boatEndAudioComponent;

    [SerializableField]
    private float midLoopTriggerTime;
    [SerializableField]
    private float endLoopTriggerTime;


    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        waterEndLoopAudioComponent.PlaySound(waterLoopSFX);
    }

    public void BeginBoatAudio()
    {
        boatStartAudioComponent.PlaySound(boatStartSFX);
        Invoke(() =>
        {
            boatStartAudioComponent.StopSound(boatStartSFX);
            boatLoopAudioComponent.PlaySound(boatLoopSFX);
        }, midLoopTriggerTime);
    }
    public void EndBoatAudio()
    {
        Invoke(() =>
        {
            boatLoopAudioComponent.StopSound(boatLoopSFX);
            waterLoopAudioComponent.StopSound(waterLoopSFX);
            boatEndAudioComponent.PlaySound(boatEndSFX);
            waterEndLoopAudioComponent.PlaySound(waterEndLoopSFX);
        }, endLoopTriggerTime);
        
    }

}