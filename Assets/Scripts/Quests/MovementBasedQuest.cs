// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class MovementBasedQuest : Quest
{
    [SerializableField]
    private GameObject checkPointIndicator;
    public override void OnSuccess(){
        Destroy(checkPointIndicator);
    }
}