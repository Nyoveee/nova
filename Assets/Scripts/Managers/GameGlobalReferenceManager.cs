// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class GameGlobalReferenceManager : Script
{
    public GameObject[] vantagePoints;
    // This function is first invoked when game starts.
    protected override void init()
    {
        vantagePoints = GameObject.FindGameObjectsWithTag("Vantage Point");
    }

    // This function is invoked every fixed update.
    protected override void update()
    {}

}