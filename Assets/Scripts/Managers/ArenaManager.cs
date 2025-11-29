// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using System.Diagnostics;

public class ArenaManager : Script
{
    // Sequential order
    [SerializableField] private List<WaveBehavior> waves;

    // True for survival, false for sequential
    [SerializableField] private bool timerMode = false;
    [SerializableField] private float survivalDuration = 60f;

    private float survivalTimer = 0f;
    private bool arenaCompleted = false;

    private List<WaveBehavior> activeWaves = new List<WaveBehavior>();
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
            WaveBehavior wave = waves[currentWave];
            activeWaves.Add(wave);
            wave.StartWave();
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

    public void OnWaveCompleted(WaveBehavior wave)
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
        foreach (WaveBehavior wave in waves)
            wave.EndWave();

        arenaCompleted = true;
        activeWaves.Clear();

        if (arenaQuest != null)
            arenaQuest.SetQuestState(Quest.QuestState.Success);

        Debug.Log("Arena Completed!");
    }
}