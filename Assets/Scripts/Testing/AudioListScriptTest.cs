// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class AudioListScriptTest : Script
{
    private AudioComponent_ audioComponent;
    public Audio audio;
    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        audioComponent = getComponent<AudioComponent_>();
        MapKey(Key._1, PlaySound1);
    }

    private void PlaySound1()
    {
        audioComponent.PlaySound(audio);
    }

}