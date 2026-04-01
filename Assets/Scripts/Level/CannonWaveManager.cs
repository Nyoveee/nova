// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using Windows.Security.Cryptography.Core;
class CannonWaveManager : Script
{
    [SerializableField]
    private List<Prefab> cannon1Prefabs;
    [SerializableField]
    private List<Prefab> cannon2Prefabs;
    [SerializableField]
    private List<Prefab> cannon3Prefabs;
    [SerializableField]
    private List<Prefab> cannon4Prefabs;
    [SerializableField]
    private GameObject endofLevel;
    [SerializableField]
    private GameObject boat;
    [SerializableField]
    private float timeCount = 120f;
    [SerializableField]
    private GameObject lever;

    private float currentTime;
    private bool waveActive = false;
    private GameObject[] cannons;

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        cannons = GameObject.FindGameObjectsWithTag("Cannon");
    }

    // This function is invoked every update.
    protected override void update()
    {
        if (lever.getScript<Switch>().isSwitchActivated() == true)
        {
            currentTime += Time.V_DeltaTime();
        }
        if (currentTime >= timeCount)
        {
            waveActive = false;
            endofLevel.getScript<EndOfLevel2>().StartScroll();
            boat.getScript<RaiseEnemBoat>().StartOutro();
        }
        else if(waveActive)
        {
            SetCannonPrefab(cannons[0], cannon1Prefabs);
            SetCannonPrefab(cannons[1], cannon2Prefabs);
            SetCannonPrefab(cannons[2], cannon3Prefabs);
            SetCannonPrefab(cannons[3], cannon4Prefabs);
        }
    }
    private void SetCannonPrefab(GameObject cannon, List<Prefab> prefabs)
    {
        float slice = timeCount / prefabs.Count;
        int index = (int)(currentTime / slice);
        cannon.getScript<EnemyCannon>().enemyPrefab = prefabs[index];
    }
    public void StartWave()
    {
        waveActive = true;
    }
    public bool IsWaveActive() => (waveActive);
}