// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;
using System;

public abstract class WaveBehavior : Script
{
    private ArenaManager arenaManager;

    private List<GameObject> aliveEnemies = new List<GameObject>();
    private bool active = false;

    protected override void init()
    {}

    // ============================================
    // Interfaces to implement..
    public virtual void StartWave(ArenaManager arenaManager)
    {
        this.arenaManager = arenaManager;

        if (arenaManager == null) {
            Debug.LogError("Failed to start wave! Arena manager is null.");
            return;
        }

        aliveEnemies.Clear();
        active = true;
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

        UpdateWave(aliveEnemies.Count);

        if (IsWaveComplete(aliveEnemies.Count))
        {
            active = false;

            if (arenaManager != null)
                arenaManager.OnWaveCompleted(this);
            else
                Debug.LogWarning("Arena not set for wave obj: " + gameObject.ToString());
        }
    }
    // ============================================

    public abstract void UpdateWave(int aliveCount);

    public abstract bool IsWaveComplete(int aliveCount);

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

 

    public void RegisterSpawn(GameObject enemy)
    {
        aliveEnemies.Add(enemy);
    }

    public int AliveCount => aliveEnemies.Count;
    public List<GameObject> GetAliveEnemies() => aliveEnemies;
}