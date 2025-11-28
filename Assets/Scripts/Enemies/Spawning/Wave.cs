// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;
using System;

public class Wave : Script
{
    public Prefab fixedSpawnPod;

    private List<SpawnPodLocation> allPodsLocation = new List<SpawnPodLocation>();

    [SerializableField]
    private ArenaManager arenaManager;
    private List<GameObject> spawnedEnemies = new List<GameObject>();

    private int enemyCount;
    private bool waveStarted = false;

    private bool hasInited = false;
    protected override void init()
    {
        // we can't just init here normally because order of operations of init is not defined.
        if (hasInited)
        {
            return;
        }
           
        hasInited = true; 

#if false
        // Turns pods inactive initially if they were not already
        foreach (SpawnPodLocation pod in fixedSpawns)
        {
            pod.gameObject.SetActive(false);
        }
#endif
        // i like the idea of using children as source of truth
        GameObject[] children = gameObject.GetChildren();

        foreach (GameObject child in children) {
            SpawnPodLocation spawnPod = child.getScript<SpawnPodLocation>();

            if (spawnPod != null)
            {
                allPodsLocation.Add(spawnPod);
            }
        }
    }

    public void StartWave()
    {
        // force init if not done.
        init();

        waveStarted = true;
        spawnedEnemies.Clear();
        enemyCount = 0;

        foreach (SpawnPodLocation pod in allPodsLocation)
        {
            GameObject createdPod = Instantiate(fixedSpawnPod, pod.gameObject.transform.position);
            FixedSpawnPod podScript = createdPod.getScript<FixedSpawnPod>();

            if (podScript != null)
            {
                podScript.Spawn(pod.enemy, this);
                ++enemyCount;
            }
            else
            {
                Debug.LogWarning("Pod prefab does not contain pod script!");
            }
        }

#if false
        if (randomSpawns != null)
        {
            foreach (RandomEnemySpawns randSpawns in randomSpawns)
            {
                enemyCount += randSpawns.SpawnEnemies();
            }
        }

        Debug.Log("Enemies: " + enemyCount);
#endif

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