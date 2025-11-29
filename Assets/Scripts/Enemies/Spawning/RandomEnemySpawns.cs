// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.

using ScriptingAPI;

public class RandomEnemySpawns : Script
{
    [SerializableField]
    private int enemyCount = 3;
    [SerializableField]
    private List<Prefab> possibleEnemyPrefabs;
    
    [SerializableField] private Prefab randomPodPrefab;
    [SerializableField] private Wave wave;
    [SerializableField] private float minSpacing = 2f;

    public int SpawnEnemies()
    {
        if (possibleEnemyPrefabs == null ||  possibleEnemyPrefabs.Count == 0) {
            Debug.LogWarning("Random enemy spawn called with no supplied prefabs.");
            return 0;
        }

        Transform_ transform = gameObject.transform;
        Vector3 spawnMin = -transform.scale * 0.25f;
        Vector3 spawnMax = transform.scale * 0.25f;

        List<Vector3> spawnedPositions = new List<Vector3>();

        for (int i = 0; i < enemyCount; i++)
        {
            Vector3 pos = Vector3.Zero();
            int attempts = 0;
            bool validPosFound = false;

            // Tries to find a valid spot
            while (attempts < 30)
            {
                attempts++;

                pos = new Vector3(
                    Random.Range(spawnMin.x, spawnMax.x),
                    transform.position.y,
                    Random.Range(spawnMin.z, spawnMax.z)
                );

                if (!IsTooClose(pos, spawnedPositions))
                {
                    validPosFound = true;
                    break;
                }
            }

            // If no valid spot found, skip spawning this enemy
            if (!validPosFound)
            {
                Debug.LogWarning("Could not find non-overlapping spawn position for enemy #" + i);
                continue;
            }

            spawnedPositions.Add(pos);

            Prefab enemyPrefab = possibleEnemyPrefabs[Random.Range(0, possibleEnemyPrefabs.Count)];

            GameObject podGO = Instantiate(randomPodPrefab, pos, gameObject);

            RandomSpawnPod pod = podGO.getScript<RandomSpawnPod>();
            if (pod != null) 
                pod.InitValues(wave, enemyPrefab);
        }
        return enemyCount;
    }

    private bool IsTooClose(Vector3 pos, List<Vector3> existing)
    {
        foreach (Vector3 entity in existing)
            if (Vector3.Distance(pos, entity) < minSpacing)
                return true;

        return false;
    }
}