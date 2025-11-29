// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;
using System;

public class FixedSpawnWaveBehavior : WaveBehavior
{
    [SerializableField] private List<FixedSpawnPod> fixedSpawns;
    [SerializableField] private List<RandomEnemySpawns> randomSpawns;

    public override void StartWave()
    {
        foreach (FixedSpawnPod pod in fixedSpawns)
        {
            // pod.Spawn();
        }

        if (randomSpawns != null)
        {
            foreach (RandomEnemySpawns randSpawn in randomSpawns)
                randSpawn.SpawnEnemies();
        }
    }

    public override void UpdateWave(int aliveCount)
    { }

    public override bool IsWaveComplete(int aliveCount)
    {
        return aliveCount <= 0;
    }
}