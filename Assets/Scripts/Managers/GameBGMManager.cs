// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using System.Runtime.CompilerServices;
using ScriptingAPI;
class GameBGMManager : Script
{
    private delegate void CurrentState();
    private enum BGMState
    {
        NonCombat,
        Transition,
        Combat,
    }
    private BGMState bgmState = BGMState.NonCombat;
    private Dictionary<BGMState, CurrentState> updateState = new Dictionary<BGMState, CurrentState>();
    private float currentTransitionTimer = 0f;
    private float currentBufferTime;
    /***********************************************************
        Components
    ***********************************************************/
    private AudioComponent_? audioComponent;
    /***********************************************************
        Inspector Variables
    ***********************************************************/
    [SerializableField]
    private float transitionTime;
    [SerializableField]
    private float bufferTime;
    [SerializableField]
    private Audio nonCombatbgm;
    [SerializableField]
    private Audio transitionBGM;
    [SerializableField]
    private Audio combatBGM;
 
    // This function is first invoked when game starts.
    protected override void init()
    {
        audioComponent = getComponent<AudioComponent_>();
        audioComponent.PlayBGM(nonCombatbgm);
        updateState.Add(BGMState.NonCombat, NonCombatState);
        updateState.Add(BGMState.Transition, TransitionState);
        updateState.Add(BGMState.Combat, CombatState);
        currentBufferTime = bufferTime;
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        updateState[bgmState]();
    }
    private bool IsInCombat()
    {
        GameObject[] gameObjects = GameObject.FindGameObjectsWithTag("Enemy");
        GameObject[] pods = GameObject.FindGameObjectsWithTag("Pod");
        if (pods.Length > 0)
            return true;
        foreach(GameObject gameObject in gameObjects)
        {
            Enemy? enemy = gameObject.getScript<Enemy>();
            if (enemy == null)
                continue;
            if (enemy.IsEngagedInBattle())
                return true;
        }
        return false;
    }
    /**********************************************************************
       Enemy States
   **********************************************************************/
    private void NonCombatState()
    {
        if (IsInCombat())
        {
            currentBufferTime -= Time.V_DeltaTime();
            if (currentBufferTime <= 0f)
            {
                bgmState = BGMState.Transition;
                audioComponent.PlaySound(transitionBGM);
                currentTransitionTimer = 0f;
            }
        }
        else
            currentBufferTime = bufferTime;
    }
    private void TransitionState()
    {
        currentTransitionTimer += Time.V_DeltaTime();
        if(currentTransitionTimer >= transitionTime){
            if (IsInCombat()){
                bgmState = BGMState.Combat;
                audioComponent.PlayBGM(combatBGM);
                currentBufferTime = bufferTime;
                return;
            }
            bgmState = BGMState.NonCombat;
            audioComponent.PlayBGM(nonCombatbgm);
        }
    }
    private void CombatState()
    {
        if (!IsInCombat())
        {
            currentBufferTime -= Time.V_DeltaTime();
            if (currentBufferTime <= 0f)
            {
                bgmState = BGMState.Transition;
                audioComponent.PlaySound(transitionBGM);
                currentTransitionTimer = 0f;
                return;
            }
        }
        else
            currentBufferTime = bufferTime;
    }
}