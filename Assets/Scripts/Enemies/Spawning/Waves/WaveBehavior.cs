// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;
using System;

public abstract class WaveBehavior : Script
{
    private ArenaManager arenaManager;

    private List<GameObject> enemies = new List<GameObject>();
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

        enemies.Clear();
        active = true;
    }

    protected override void update()
    {
        if (!active)
            return;

        for (int i = enemies.Count - 1; i >= 0; i--)
        {
            if (enemies[i] == null) // enemy died / destroyed
            {
                enemies.RemoveAt(i);
            }
        }

        UpdateWave(enemies.Count);

        if (IsWaveComplete(enemies))
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

    public abstract bool IsWaveComplete(List<GameObject> aliveEnemies);

    public void ResetWave()
    {
        active = false;
        foreach (GameObject enemy in enemies)
        {
            if (enemy != null)
                Destroy(enemy);
        }
        enemies.Clear();
    }
    public void EndWave()
    {
        active = false;
    }

 

    public void RegisterSpawn(GameObject enemy)
    {
        enemies.Add(enemy);
    }

    public int AliveCount => enemies.Count;
}