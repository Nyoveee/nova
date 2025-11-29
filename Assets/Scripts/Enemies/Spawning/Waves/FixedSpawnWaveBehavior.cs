// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;
using System;

public class FixedSpawnWaveBehavior : WaveBehavior
{
    public Prefab fixedPodScript;
    private List<SpawnPodLocation> podLocations = new List<SpawnPodLocation>();

    public override void StartWave(ArenaManager arenaManager)
    {
        base.StartWave(arenaManager);

        // populate our pod locations..
        foreach (GameObject podLocation in gameObject.GetChildren()) {
            SpawnPodLocation podLocationScript = podLocation.getScript<SpawnPodLocation>();

            if (podLocationScript != null) { 
                podLocations.Add(podLocationScript);
            }
        }

        foreach (SpawnPodLocation pod in podLocations)
        {
            GameObject createdPod = Instantiate(fixedPodScript, pod.gameObject.transform.position);
            FixedSpawnPod podScript = createdPod.getScript<FixedSpawnPod>();

            if (podScript != null && pod.enemies.Count != 0)
            {
                podScript.Spawn(pod.enemies[0], this);
            }
            else
            {
                Debug.LogWarning("Pod prefab does not contain pod script or spawn pod location list is empty!");
            }
        }
    }

    public override void UpdateWave(int aliveCount)
    { }

    public override bool IsWaveComplete(int aliveCount)
    {
        return aliveCount <= 0;
    }
}