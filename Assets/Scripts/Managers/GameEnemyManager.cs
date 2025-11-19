// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class GameEnemyManager : Script
{
    // Serialized fields..
    public required Transform_ spawningArea;
    public required Prefab enemyPrefab;

    public required float minSpawnInterval = 5f;
    public required float maxSpawnInterval = 10f;

    // Runtime variables..
    private float nextSpawnInterval;
    private float timeElapsed;

    private System.Random random = new System.Random();

    // This function is first invoked when game starts.
    protected override void init()
    {
        nextSpawnInterval = GetNextSpawnInterval();
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        if(timeElapsed > nextSpawnInterval)
        {
            nextSpawnInterval = GetNextSpawnInterval();
            SpawnEnemy();
            timeElapsed -= nextSpawnInterval;
        }

        timeElapsed += Time.V_FixedDeltaTime();
    }

    private float GetNextSpawnInterval()
    {
        return Mathf.Interpolate(minSpawnInterval, maxSpawnInterval, (float) random.NextDouble(), 1f);
    }

    private void SpawnEnemy()
    {
        // Get a random spot in the spawning area..
        // We use the scale of the spawning area as the width and height of the spawning area.
        // Spawning area size is defined by a 1 unit cube scaled accordingly.
        float xOffset = (float)(random.NextDouble() - 0.5) * spawningArea.scale.x;
        float zOffset = (float)(random.NextDouble() - 0.5) * spawningArea.scale.z;

        Vector3 spawnPosition = new Vector3(spawningArea.position.x + xOffset, spawningArea.position.y, spawningArea.position.z + zOffset);
        Instantiate(enemyPrefab, spawnPosition, null);
    }
}