// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
public class ArenaManager : Script
{
    [SerializableField] 
    private List<Wave> waves;
    //[SerializableField]
    //private List<Door> doors;
    //[SerializableField]
    //private List<Door::States> arenaCompletionDoorStates = new List<Door::States::Open>

    private int currentWave = 0;

    protected override void init()
    {
        // TEMP MAKE IT SOMETHING ELSE
        StartArena();
    }

    public void StartArena()
    {
        currentWave = 0;
        waves[currentWave].StartWave();
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
    }

    public void ArenaCompleted()
    {
        Debug.Log("Arena Completed!");
    }
}