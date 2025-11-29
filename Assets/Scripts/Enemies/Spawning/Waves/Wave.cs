// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;
using System;
using System.Diagnostics;

public class Wave : Script
{
#if false
    private WaveBehavior behavior;
    [SerializableField] private ArenaManager arenaManager;

    private List<GameObject> aliveEnemies = new List<GameObject>();
    private bool active = false;

    protected override void init()
    {
        behavior = getScript<WaveBehavior>();
        if (behavior != null) 
            behavior.Init(this);
    }

    public void StartWave()
    {
        aliveEnemies.Clear();
        active = true;
        if (behavior != null)
            behavior.StartWave();
    }
    public void EndWave()
    {
        active = false;
        foreach (GameObject enemy in aliveEnemies)
        {
            if (enemy != null)
                Destroy(enemy);
        }
        aliveEnemies.Clear();
    }


    protected override void update()
    {
        if (!active) 
            return;

        for (int i = aliveEnemies.Count - 1; i >= 0; i--)
        {
            if (aliveEnemies[i] == null) // enemy died / destroyed
            {
                aliveEnemies.RemoveAt(i);
            }
        }

        if (behavior == null)
            return;

        behavior.UpdateWave(aliveEnemies.Count);

        if (behavior.IsWaveComplete(aliveEnemies.Count))
        {
            active = false;
            if (arenaManager != null)
                arenaManager.OnWaveCompleted(this);
            else
                Debug.LogWarning("Arena not set for wave obj: " + gameObject.ToString());
        }
    }

    public void RegisterSpawn(GameObject enemy)
    {
        aliveEnemies.Add(enemy);
    }

    public int AliveCount => aliveEnemies.Count;
    public List<GameObject> GetAliveEnemies() => aliveEnemies;
#endif
}