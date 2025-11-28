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

    public int SpawnEnemies()
    {
        if (possibleEnemyPrefabs == null ||  possibleEnemyPrefabs.Count == 0) {
            Debug.LogWarning("Random enemy spawn called with no supplied prefabs.");
            return 0;
        }

        Transform_ transform = gameObject.transform;
        Vector3 spawnMin = -transform.scale * 0.5f;
        Vector3 spawnMax = transform.scale * 0.5f;

        for (int i = 0; i < enemyCount; i++)
        {
            Vector3 pos = new Vector3(
                            Random.Range(spawnMin.x, spawnMax.x),
                            transform.position.y,
                            Random.Range(spawnMin.z, spawnMax.z));

            // TODO: ENEMIES CAN SPAWN WITHIN EACH OTHER AT THE MOMENT 
            Prefab enemyPrefab = possibleEnemyPrefabs[Random.Range(0, possibleEnemyPrefabs.Count)];

            GameObject podGO = Instantiate(randomPodPrefab, pos, gameObject);

            RandomSpawnPod pod = podGO.getScript<RandomSpawnPod>();
            if (pod != null) 
                pod.InitValues(wave, enemyPrefab);
        }
        return enemyCount;
    }
}