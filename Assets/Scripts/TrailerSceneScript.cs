// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class TrailerSceneScript : Script
{
    [SerializableField]
    private Scene sceneToChange;
    [SerializableField]
    private Audio videoAudio;
    private AudioComponent_ audioComponent;
    private VideoPlayer_ videoPlayer;

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        audioComponent = getComponent<AudioComponent_>();
        videoPlayer = getComponent<VideoPlayer_>();
        audioComponent.PlaySound(videoAudio);
    }
    protected override void update()
    {
        if (videoPlayer.IsVideoFinished())
            SceneAPI.ChangeScene(sceneToChange);
    }

}