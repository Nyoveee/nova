// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class PlaySoundOnStart : Script
{
    public Audio audio;
    public float delay = 0f;

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        Invoke(() =>
        {
            getComponent<AudioComponent_>().PlaySound(audio);
        }, delay);   
    }

    // This function is invoked every update.
    protected override void update()
    {}

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

}