// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class CannonWaveManager : Script
{
    [SerializableField] 
    private GameObject[] cannons;
    [SerializableField] 
    private int enemiesPerWave = 2;
    [SerializableField] 
    private int totalWaves = 3;
    [SerializableField]
    private GameObject endofLevel;
    [SerializableField]
    private GameObject boat;
    [SerializableField]
    private GameObject boat2;

    private int currentWave = 0;
    private bool waveActive = false;
    private int pendingEnemies = 0;
    private List<GameObject> activeEnemies = new List<GameObject>();

    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        cannons = GameObject.FindGameObjectsWithTag("Cannon");
    }

    // This function is invoked every update.
    protected override void update()
    {
        activeEnemies.RemoveAll(e => e == null || e.getScript<Enemy>().IsDead());
        //if (!waveActive && currentWave < totalWaves)
        //{
        //    waveActive = true;
        //    FireWave();
        //}

        // If wave is active, check if all enemies are dead
        if (waveActive && pendingEnemies == 0 && activeEnemies.Count == 0)
        {
            waveActive = false;
            currentWave++;
            Debug.Log($"Wave {currentWave} finished");

            if (currentWave < totalWaves)
            {
                waveActive = true;
                FireWave();
                Debug.Log($"Wave {currentWave + 1} started");
            }
            else
            {
                Debug.Log("All waves completed");
                endofLevel.getScript<EndOfLevel2>().StartScroll();
                boat.getScript<RaiseEnemBoat>().Sink();
                boat2.getScript<RaiseEnemBoat>().Sink();
            }
        }
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}

    // This function is invoked when destroyed.
    protected override void exit()
    {}

    private void FireWave()
    {
        int cannonIndex = 0;
        for (int i = 0; i < enemiesPerWave; i++)
        {
            pendingEnemies++;
            cannons[cannonIndex].getScript<EnemyCannon>().FireNextShot();
            cannonIndex = (cannonIndex + 1) % cannons.Length; // round-robin across cannons
        }
    }

    public void StartWave()
    {
        if (currentWave == 0 && !waveActive)
        {
            waveActive = true;
            FireWave();
        }
    }

    public void RegisterEnemy(GameObject enemy)
    {
        pendingEnemies--;
        activeEnemies.Add(enemy);
    }

    public void RemoveEnemy(GameObject enemy)
    {
        activeEnemies.Remove(enemy);
    }

    public int ActiveEnemyCount()
    {
        return activeEnemies.Count;
    }

    public bool HasActiveEnemies()
    {
        return activeEnemies.Count > 0;
    }
}