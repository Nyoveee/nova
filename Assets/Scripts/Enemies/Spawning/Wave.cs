// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;
using System;

public class Wave : Script
{
    public Prefab fixedSpawnPod;

    [SerializableField] 
    private List<SpawnPodLocation> fixedSpawns;

    [SerializableField]
    private List<RandomEnemySpawns>? randomSpawns;

    [SerializableField]
    private ArenaManager arenaManager;
    private List<GameObject> spawnedEnemies = new List<GameObject>();

    private int enemyCount;
    private bool waveStarted = false;

    protected override void init()
    {
        // we can't just init here because order of operations of init is not defined.
        // we utilise serialised fields to ensure that the field is set.

        // arenaManager = gameObject.GetParent().getScript<ArenaManager>();

#if false
        // Turns pods inactive initially if they were not already
        foreach (SpawnPodLocation pod in fixedSpawns)
        {
            pod.gameObject.SetActive(false);
        }
#endif
    }

    public void StartWave()
    {
        waveStarted = true;
        spawnedEnemies.Clear();
        enemyCount = 0;

        foreach (SpawnPodLocation pod in fixedSpawns)
        {
            GameObject createdPod = Instantiate(fixedSpawnPod, pod.gameObject.transform.position);
            FixedSpawnPod podScript = createdPod.getScript<FixedSpawnPod>();

            if (podScript != null)
            {
                podScript.Spawn(pod.enemy, this);
            }
            else
            {
                Debug.LogWarning("Pod prefab does not contain pod script!");
            }
        }

        if (randomSpawns != null)
        {
            foreach (RandomEnemySpawns randSpawns in randomSpawns)
            {
                enemyCount += randSpawns.SpawnEnemies();
            }
        }

        Debug.Log("Enemies: " + enemyCount);
        if (enemyCount == 0)
        {
            Debug.LogWarning("Wave object " + gameObject.ToString() + " started with no enemies");
            arenaManager.OnWaveCompleted();
        }
    }

    public void RegisterSpawn(GameObject enemy)
    {
        spawnedEnemies.Add(enemy);
    }

    public void EndWave()
    {
        foreach (GameObject enemy in spawnedEnemies)
        {
            if (enemy != null)
                Destroy(enemy);
        }

        spawnedEnemies.Clear();
        waveStarted = false;
    }

    protected override void update()
    {
        if (!waveStarted)
            return;

        // Check if enemies are destroyed
        for (int i = spawnedEnemies.Count - 1; i >= 0; i--)
        {
            if (spawnedEnemies[i] == null)
            {
                spawnedEnemies.RemoveAt(i);
                enemyCount--;
                Debug.Log("ENEMY REMOVED");

                if (enemyCount <= 0)
                {
                    EndWave();
                    arenaManager.OnWaveCompleted();
                }
            }
        }
    }
}