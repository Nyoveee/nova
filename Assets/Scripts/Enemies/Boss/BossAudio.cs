// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

class BossAudio : Script
{
    [SerializableField]
    public List<Audio> walkSoundsAudio;

    [SerializableField]
    public int footStepIndex = 0;

    [SerializableField]
    public List<Audio> fireRocketAudio;

    [SerializableField]
    public List<Audio> shockWaveAudio;

    [SerializableField]
    public List<Audio> meleeAttackAudio;

    [SerializableField]
    public List<Audio> bossHitAudio;



    // This function is invoked once before init when gameobject is active.
    //protected override void awake()
    //{}

    //// This function is invoked once when gameobject is active.
    //protected override void init()
    //{}

    //// This function is invoked every update.
    //protected override void update()
    //{}

    //// This function is invoked every update.
    //protected override void fixedUpdate()
    //{}

    //// This function is invoked when destroyed.
    //protected override void exit()
    //{}

}