// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class Admin : Script
{
    private PlayerController_V2 playerBody;

    private Sniper sniper;

    [SerializableField]
    private QuestManager questManager;
    // This function is invoked once before init when gameobject is active.
    protected override void awake()
    {}

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        playerBody = GameObject.FindWithTag("Player").getScript<PlayerController_V2>();
        sniper = GameObject.FindWithTag("Sniper").getScript<Sniper>();

        // instant death.
        MapKey(Key.K, () =>
        {
            playerBody?.TakeDamage(1000);
        });

        // closure..
        bool isSpedUp = false;

        // speed hack
        MapKey(Key.L, () =>
        {
            isSpedUp = !isSpedUp;
            
            if(isSpedUp)
            {
                Time.timeScale = 5;
            }
            else
            {
                Time.timeScale = 1;
            }

        });

        // Skip Quest
        MapKey(Key.O, () =>
        {
            questManager?.SkipCurrentQuest();
        });

        // Take 30 damage..
        MapKey(Key.I, () =>
        {
            playerBody?.TakeDamage(30);
        });

        // Heal 10 damage..
        MapKey(Key.U, () =>
        {
            playerBody?.GainHealth(10f);
        });

        MapKey(Key.Y, () =>
        {
            // playerBody?.GainHealth(10f);
            sniper.CurrentSp += 17;
        });
    }
}