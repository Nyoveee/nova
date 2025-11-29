// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;
using System;

// Spawns repeatedly with wave interval unless the capped enemies are hit
public class ConstantCapWaveBehavior : WaveBehavior
{
    [SerializableField] private List<RandomEnemySpawns> randomSpawns;
    [SerializableField] private int maxAlive = 10;
    [SerializableField] private float spawnInterval = 2f;

    private float timer = 0f;

    public override void StartWave()
    {
        SpawnEnemies();
    }

    void SpawnEnemies()
    {
        foreach (RandomEnemySpawns randSpawn in randomSpawns)
        {
            randSpawn.SpawnEnemies();
        }
    }

    public override void UpdateWave(int aliveCount)
    {
        timer += Time.V_DeltaTime();

        if (aliveCount < maxAlive && timer >= spawnInterval)
        {
            timer = 0f;
            SpawnUnlessCap(aliveCount);
        }
    }

    void SpawnUnlessCap(int aliveCount)
    {
        foreach (RandomEnemySpawns randSpawn in randomSpawns)
        {
            if (aliveCount >= maxAlive)
                return;

            randSpawn.SpawnEnemies();
        }
    }

    public override bool IsWaveComplete(int aliveCount)
    {
        // Never completed, handled by arena 
        return false;
    }
}