// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class GameGlobalReferenceManager : Script
{
    public List<GameObject> vantagePoints;
    // This function is first invoked when game starts.
    protected override void init()
    {
        foreach(GameObject vantagePoint in GameObject.FindGameObjectsWithTag("Vantage Point"))
        {
            if(vantagePoint.IsActive())
            {
                vantagePoints.Add(vantagePoint);
            }
        }

#if false
        foreach (GameObject vantagePoint in vantagePoints) {
            Debug.Log(vantagePoint.transform.position);
        }
#endif
    }

    // This function is invoked every fixed update.
    protected override void update()
    {}

}