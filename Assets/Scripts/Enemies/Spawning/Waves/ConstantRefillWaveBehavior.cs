// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;
using System;

// Constantly refills enemies upon all enemies in the lists being destroyed
public class ConstantRefillWaveBehavior : WaveBehavior
{
    [SerializableField] private List<RandomEnemySpawns> randomSpawns;

    public override void StartWave(ArenaManager arenaManager)
    {
        base.StartWave(arenaManager);
        SpawnEnemies();
    }

    void SpawnEnemies()
    {
        foreach (RandomEnemySpawns randSpawn in randomSpawns)
            randSpawn.SpawnEnemies();
    }

    public override void UpdateWave(int aliveCount)
    {
        // Only spawns if all objects attached to this behavior is dead
        if (aliveCount == 0)
            SpawnEnemies();
    }

    public override bool IsWaveComplete(int aliveCount)
    {
        // Never completed, handled by arena 
        return false;
    }
}