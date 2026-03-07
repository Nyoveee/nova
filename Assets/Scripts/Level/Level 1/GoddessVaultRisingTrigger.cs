// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using Windows.Gaming.Input.ForceFeedback;

class GoddessVaultRisingTrigger : Script
{
    [SerializableField]
    private GameObject goddessGameObject;
    [SerializableField]
    private Vector3 startPoint;
    [SerializableField]
    private Vector3 endPoint;

    protected override void onCollisionEnter(GameObject other)
    {
        if(other.tag == "Player")
        {
            goddessGameObject.getScript<GoddessBehaviour>().BeginRising(startPoint, endPoint);
            Destroy(gameObject);
        }
            
    }

}