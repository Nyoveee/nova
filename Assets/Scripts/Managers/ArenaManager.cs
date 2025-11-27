// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using System.Diagnostics;

public class ArenaManager : Script
{
    [SerializableField] 
    private List<Wave> waves;
    //[SerializableField]
    //private List<Door> doors;
    //[SerializableField]
    //private List<Door::States> arenaCompletionDoorStates = new List<Door::States::Open>

    private int currentWave = 0;
    private ArenaQuest arenaQuest;

    public void StartArena(ArenaQuest quest)
    {
        currentWave = 0;
        arenaQuest = quest;
        if (waves != null && waves.Count > 0) 
            waves[currentWave].StartWave();
        else
        {
            Debug.LogWarning("Arena " + gameObject.ToString() + " is missing waves");
            ArenaCompleted();
        }
    }

    public void OnWaveCompleted()
    {
        currentWave++;

        if (currentWave < waves.Count) {
            waves[currentWave].StartWave();
        }
        else
            ArenaCompleted();
    }

    public void ResetArena()
    {
        foreach (var wave in waves)
            wave.EndWave();

        currentWave = 0;
        waves[currentWave].StartWave();
    }

    public void ArenaCompleted()
    {
        Debug.Log("Arena Completed!");
        if (arenaQuest != null)
            arenaQuest.SetQuestState(Quest.QuestState.Success);
    }
}