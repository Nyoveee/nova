// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using Windows.Gaming.Input.ForceFeedback;

class GoddessVaultRisingTrigger : Script
{
    [SerializableField]
    private GameObject goddessGameObject;
    [SerializableField]
    private Vector3 startPoint;
    [SerializableField]
    private Vector3 endPoint;

    [SerializableField]
    private List<string> goddessVoiceOverText;
    [SerializableField]
    private List<float> goddessVoiceOverTime;
    [SerializableField]
    private Audio goddessVoiceOverAudio;
    protected override void onCollisionEnter(GameObject other)
    {
        if(other.tag == "Player")
        {
            goddessGameObject.getScript<GoddessBehaviour>()?.BeginRising(startPoint, endPoint);
            goddessGameObject.getScript<GoddessBehaviour>()?.SetFloatingSpeech(goddessVoiceOverText, goddessVoiceOverTime, goddessVoiceOverAudio);
            Destroy(gameObject);
        }
            
    }

}