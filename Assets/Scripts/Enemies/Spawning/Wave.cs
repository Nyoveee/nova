// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;
using System;

public class Wave : Script
{
    // Prefab pod;

    [SerializableField]
    private List<Transform_>? fixedEnemyPos;
    [SerializableField]
    private List<Prefab>? fixedEnemyPrefabs;
    [SerializableField]
    private List<RandomEnemySpawns>? randomSpawns;

    private int enemyCount;
    private ArenaManager? arenaManager;
    private List<GameObject> spawnedEnemies = new List<GameObject>();
    private bool waveStarted = false;

    protected override void init()
    {
        arenaManager = gameObject.GetParent().getScript<ArenaManager>();
        enemyCount = 0;
    }

    public void StartWave()
    {
        waveStarted = true;
        spawnedEnemies.Clear();

        // Unsure how to do make this better rn
        if (fixedEnemyPos.Count != fixedEnemyPrefabs.Count)
        {
            Debug.LogError("Enemy positions dont match number of prefabs");
        }

        for (int i = 0; i < fixedEnemyPrefabs.Count; i++)
        {
            enemyCount++;
            // TEMP TILL SPAWN PODS EXIST
            GameObject enem = Instantiate(fixedEnemyPrefabs[i], fixedEnemyPos[i].position, gameObject);
            RegisterSpawn(enem);
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