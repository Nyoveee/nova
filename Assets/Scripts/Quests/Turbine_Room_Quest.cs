// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class Turbine_Room_Quest : Quest
{
    [SerializableField]
    private List<Switch> switches = new List<Switch>();

    [SerializableField]
    private Prefab gunnerPrefab;

    [SerializableField]
    private List<GameObject> gunnerSpawnLocations1;

    [SerializableField]
    private List<GameObject> gunnerSpawnLocations2;

    [SerializableField]
    private List<GameObject> gunnerSpawnLocations3;

    [SerializableField]
    private Door turbineExitDoor;

    [SerializableField]
    private float questCompleteDelay = 1f;

    [SerializableField]
    private string goddessVoiceOverText;
    [SerializableField]
    private float goddessVoiceOverTime;
    [SerializableField]
    private Audio goddessVoiceOverAudio;

    [SerializableField]
    private List<string> weaverVoiceOverText;
    [SerializableField]
    private List<float> weaverVoiceOverTime;
    [SerializableField]
    private List<Audio> weaverVoiceOverAudio;


    private List<List<GameObject>> spawnLocations = new List<List<GameObject>>();
    private List<GameObject> spawnedEnemies = new List<GameObject>();
    private List<Switch> activeSwitches = new List<Switch>();
    private VoiceoverScript voiceoverScript;
    private int weaverVoiceOverIndex = -1;

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        voiceoverScript = GameObject.FindWithTag("Game UI Manager")?.getScript<VoiceoverScript>();
        OnRestart();
    }
    public override void OnEnter()
    {
        voiceoverScript.TriggerVoiceOver("Goddess", goddessVoiceOverText, goddessVoiceOverAudio, goddessVoiceOverTime, false);
    }
    public override void UpdateQuest()
    {
        for (int i = activeSwitches.Count - 1; i >= 0; --i)
        {
            if (!activeSwitches[i].isSwitchActivated())
                continue;
            foreach (GameObject spawnLocation in spawnLocations[spawnLocations.Count-1])
                spawnedEnemies.Add(Instantiate(gunnerPrefab, spawnLocation.transform.position));
            ++weaverVoiceOverIndex;
            voiceoverScript.TriggerVoiceOver("Weaver", weaverVoiceOverText[weaverVoiceOverIndex], weaverVoiceOverAudio[weaverVoiceOverIndex], weaverVoiceOverTime[weaverVoiceOverIndex], false);
            spawnLocations.RemoveAt(spawnLocations.Count-1);
            activeSwitches.RemoveAt(i);
        }
        if (activeSwitches.Count > 0)
            return;
        if (!IsAllEnemiesDead())
            return;
        // at this point all switches are turned on.
        Invoke(() =>
        {
            SetQuestState(QuestState.Success);
            turbineExitDoor.UnlockDoor();

        }, questCompleteDelay);
    }
    public override void OnRestart()
    {
        foreach (GameObject spawnEnemy in spawnedEnemies)
        {
            if (spawnEnemy != null)
                Destroy(spawnEnemy);
        }
        spawnedEnemies.Clear();
        spawnLocations.Clear();
        spawnLocations.Add(gunnerSpawnLocations3);
        spawnLocations.Add(gunnerSpawnLocations2);
        spawnLocations.Add(gunnerSpawnLocations1);
        activeSwitches.Clear();
        foreach (Switch switch_ in switches)
        { 
            switch_.deactivateSwitch();
            activeSwitches.Add(switch_);
           
        }
    }
    private bool IsAllEnemiesDead()
    {
        foreach (GameObject gunner in spawnedEnemies)
        {
            if (gunner == null)
                continue;
            Enemy enemy = gunner.getScript<Enemy>();
            if (!enemy.IsDead())
                return false;
        }
        return true;
    }
}