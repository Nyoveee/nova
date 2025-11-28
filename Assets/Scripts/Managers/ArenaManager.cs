// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using System.Diagnostics;

public class ArenaManager : Script
{
    // Sequential order
    [SerializableField] private List<Wave> waves;
    // True for survival, false for sequential
    [SerializableField] private bool timerMode = false;
    [SerializableField] private float survivalDuration = 60f;

    private float survivalTimer = 0f;
    private bool arenaCompleted = false;

    private List<Wave> activeWaves = new List<Wave>();
    private int currentWave = 0;
    private ArenaQuest arenaQuest;

    public void StartArena(ArenaQuest quest)
    {
        arenaQuest = quest;

        activeWaves.Clear();
        currentWave = 0;
        survivalTimer = 0;
        arenaCompleted = false;

        if (waves != null && waves.Count > 0)
        {
            if (!timerMode)
            {
                StartNextSequentialWave();
            }
            else
            {
                StartAllSurvivalWaves();
            }
        }
        else
        {
            Debug.LogWarning("Arena " + gameObject.ToString() + " is missing waves");
            ArenaCompleted();
        }
    }

    void StartNextSequentialWave()
    {
        if (currentWave < waves.Count)
        {
            Wave w = waves[currentWave];
            activeWaves.Add(w);
            w.StartWave();
        }
        else
        {
            ArenaCompleted();
        }
    }

    void StartAllSurvivalWaves()
    {
        foreach (var w in waves)
        {
            activeWaves.Add(w);
            w.StartWave();
        }
    }

    protected override void update()
    {
        if (arenaCompleted)
            return;

        if (timerMode)
        {
            survivalTimer += Time.V_DeltaTime();

            if (survivalTimer >= survivalDuration)
                ArenaCompleted();
        }
    }

    public void OnWaveCompleted(Wave wave)
    {
        activeWaves.Remove(wave);

        if (!timerMode)
        {
            currentWave++;
            StartNextSequentialWave();
        }
    }

    public void ResetArena()
    {
        foreach (var w in activeWaves)
            w.EndWave();

        StartArena(arenaQuest);
    }

    public void ArenaCompleted()
    {
        foreach (Wave wave in waves)
            wave.EndWave();

        arenaCompleted = true;
        activeWaves.Clear();

        if (arenaQuest != null)
            arenaQuest.SetQuestState(Quest.QuestState.Success);
        Debug.Log("Arena Completed!");
    }
}