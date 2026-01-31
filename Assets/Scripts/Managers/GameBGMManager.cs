// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using System.Runtime.CompilerServices;

class GameBGMManager : Script
{
    private delegate void CurrentState();
    private enum BGMState
    {
        Normal,
        EndLevel,
        NonCombat,
        Transition,
        Combat,
    }
    private BGMState bgmState = BGMState.Normal;
    private Dictionary<BGMState, CurrentState> updateState = new Dictionary<BGMState, CurrentState>();
    private float currentTransitionTimer = 0f;
    private float currentBufferTime;
    private AudioComponent_ audioComponent;
    /***********************************************************
        Inspector Variables
    ***********************************************************/
    [SerializableField]
    private float transitionTime;
    [SerializableField]
    private float bufferTime;
    [SerializableField]
    private Audio bgmTrack1;
    [SerializableField]
    private Audio endTrackTransitBGM;


    // This function is first invoked when game starts.
    protected override void init()
    {
        audioComponent = getComponent<AudioComponent_>();
        //Play 'Vestigal_Perc-Action_BGM_Loop_140bpm'
        audioComponent.PlayBGM(bgmTrack1);
        updateState.Add(BGMState.Normal, NormalState);
        updateState.Add(BGMState.EndLevel, EndLevelState);
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
    private bool IsLevelEnd()
    {
        GameObject[] gameObjects = GameObject.FindGameObjectsWithTag("Enemy");
        foreach(GameObject gameObject in gameObjects)
        {
            Enemy? enemy = gameObject.getScript<Enemy>();
            if (enemy == null)
                continue;
        }
        return true;
    }
    /**********************************************************************
       Enemy States
   **********************************************************************/
    private void NormalState()
    {
        if(!IsLevelEnd())
        {
            currentBufferTime -= Time.V_DeltaTime();
            if(currentBufferTime <= 0f)
            {
                bgmState = BGMState.EndLevel;
                currentTransitionTimer = 0f;
            }
        }
        else
            currentBufferTime = bufferTime;
        //transition to EndLevelState at the end of level
    }
    private void EndLevelState()
    {
        //to be updated to at end of level
        if (!IsInCombat())
        {
            currentBufferTime -= Time.V_DeltaTime();
            if (currentBufferTime <= 0f)
            {
                bgmState = BGMState.Transition;
                //Play 'Vestigal_Perc-Action_End-Hit_Fire-Every_1.714sec_From-Loop-Start_140bpm' at end of action/level
                audioComponent.PlayBGM(endTrackTransitBGM);
                currentTransitionTimer = 0f;
                return;
            }
        }
        else
            currentBufferTime = bufferTime;
    }
    private void NonCombatState()
    {
        if (IsInCombat())
        {
            currentBufferTime -= Time.V_DeltaTime();
            if (currentBufferTime <= 0f)
            {
                bgmState = BGMState.Transition;
                //Play 'Vestigal_Perc-Action_BGM_Loop_140bpm'
                
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
                currentBufferTime = bufferTime;
                return;
            }
            bgmState = BGMState.NonCombat;
            //AudioAPI.PlayBGM(gameObject, "BGM_Vestigial_Nu_BGM-1st-Part_Loop_140bpm");
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
                //Play 'Vestigal_Perc-Action_End-Hit_Fire-Every_1.714sec_From-Loop-Start_140bpm' at end of action/level
                audioComponent.PlayBGM(endTrackTransitBGM);
                currentTransitionTimer = 0f;
                return;
            }
        }
        else
            currentBufferTime = bufferTime;
    }
}