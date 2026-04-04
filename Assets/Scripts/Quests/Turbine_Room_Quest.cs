// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class Turbine_Room_Quest : Quest
{
    [SerializableField]
    private List<Switch> activeSwitches = new List<Switch>();

    [SerializableField]
    private Prefab gunnerPrefab;

    [SerializableField]
    private Prefab gruntPrefab;

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
    private VoiceoverScript voiceoverScript;
    private int weaverVoiceOverIndex = -1;

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        voiceoverScript = GameObject.FindWithTag("Game UI Manager")?.getScript<VoiceoverScript>();
        spawnLocations.Add(gunnerSpawnLocations1);
        spawnLocations.Add(gunnerSpawnLocations2);
        spawnLocations.Add(gunnerSpawnLocations3);
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

            if (activeSwitches[i].gameObject.tag == "SwitchGround")
            {
                foreach (GameObject spawnLocation in spawnLocations[0])
                {
                    if (spawnLocation.tag == "Gunner")
                    {
                        spawnedEnemies.Add(Instantiate(gunnerPrefab, spawnLocation.transform.position));
                    }
                    else if (spawnLocation.tag == "Grunt")
                    {
                        spawnedEnemies.Add(Instantiate(gruntPrefab, spawnLocation.transform.position));
                    }
                }
            }

            if (activeSwitches[i].gameObject.tag == "SwitchCenter")
            {
                foreach (GameObject spawnLocation in spawnLocations[1])
                {
                    if (spawnLocation.tag == "Gunner")
                    {
                        spawnedEnemies.Add(Instantiate(gunnerPrefab, spawnLocation.transform.position));
                    }
                    else if (spawnLocation.tag == "Grunt")
                    {
                        spawnedEnemies.Add(Instantiate(gruntPrefab, spawnLocation.transform.position));
                    }
                }
            }

            if (activeSwitches[i].gameObject.tag == "SwitchTop")
            {
                foreach (GameObject spawnLocation in spawnLocations[2])
                {
                    if (spawnLocation.tag == "Gunner")
                    {
                        spawnedEnemies.Add(Instantiate(gunnerPrefab, spawnLocation.transform.position));
                    }
                    else if (spawnLocation.tag == "Grunt")
                    {
                        spawnedEnemies.Add(Instantiate(gruntPrefab, spawnLocation.transform.position));
                    }
                }
            }



            ++weaverVoiceOverIndex;
            voiceoverScript.TriggerVoiceOver("Weaver", weaverVoiceOverText[weaverVoiceOverIndex], weaverVoiceOverAudio[weaverVoiceOverIndex], weaverVoiceOverTime[weaverVoiceOverIndex], false);
            //spawnLocations.RemoveAt(spawnLocations.Count-1);
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
    public override void OnSkip()
    {
        turbineExitDoor.LockDoor();
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