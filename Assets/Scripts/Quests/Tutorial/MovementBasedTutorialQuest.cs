// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class MovementBasedTutorialQuest : Quest
{
    [SerializableField]
    private GameObject checkPointIndicator;
    // This function is first invoked when game starts.
    public override void OnEnter(){}

    public override void OnSuccess(){
        Destroy(checkPointIndicator);
    }
    public override void OnFail(Transform_ playerTransform){
        if (playerTransform != null && playerCheckpoint != null)
            playerTransform.position = playerCheckpoint.position;
    }

    public override void UpdateQuest(){ }

}