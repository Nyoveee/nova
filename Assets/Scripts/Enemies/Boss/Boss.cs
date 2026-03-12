// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using System; 
using System.Collections.Generic; 

public class Boss : Enemy
{


    /***********************************************************
    Inspector Variables
    ***********************************************************/
    [SerializableField]
    private EnemyStats bossStats = null;
    [SerializableField]
    private float abilityCoolDownTime = 2f; //boss will spam abilities until exausted or no more abilities to spam then it will refresh its currentdeck based on condition such as hp state
    [SerializableField]
    private int maxStamina = 5;
    [SerializableField]
    private float spawningDuration = 8f;
    [SerializableField]
    private float rotationSpeed = 120f;
    [SerializableField]
    private float lookAngle = 30f;
    [SerializableField]
    private float startLookTolerance = 30f; //add a buffer to rotation angle so it does not immediately jerk on plaeyer moves
    [SerializableField]
    private float stopDistance = 40f;
    [SerializableField]
    private float startDistanceTolerance = 20f; //add a buffer to start distance so it does not immediately jerk on plaeyer moves

    /***********************************************************
    Position Variables
    ***********************************************************/
    [SerializableField]
    private GameObject mainLauncher = null;
    [SerializableField]
    private List<GameObject> sideLauncher = null;
    [SerializableField]
    private GameObject meleeAttackPosition = null;

    /***********************************************************
    Ability Prefabs
    ***********************************************************/
    [SerializableField]
    private Prefab shockwavePrefab = null;
    [SerializableField]
    private Prefab missilePrefab = null;
    [SerializableField]
    private Prefab meleeWavePrefab = null;
    [SerializableField]
    private Prefab pushFieldPrefab = null;

    /***********************************************************
    Ability Prefabs
    ***********************************************************/
    [SerializableField]
    private Animator_ bossAnimator = null;

    /***********************************************************
    Private Variables (made public cause of ability sequencer)
    ***********************************************************/
    //private float accumulatedDamageInstance = 0;
    public BossState currentState = BossState.Spawning;
    public int currentStamina;
    public bool terminateExecution = false;
    public int sequenceIndexer = -1;
    public int abilityIndexer = -1;
    private float abilitytimeElapsed = 0;
    private float cooldowntimeElapsed = 0;
    private bool isIdleAnimation = false;
    Blackboard blackboard = new Blackboard(); //a blackboard helper class since i realised we gonna need to pass a lot of data around and i dont feel like creating 100 variables

    Vector3 halfExtent; //based on scaling values

    private BossUI bossUI;
    private float maxHealth;

    //The idea is that the boss can have a deck of abilities like a card game, which he can use. 
    //WHen he uses an ability, he will remove it from the deck. it cannot be used until deck is exhausted or he chooses to refresh it.
    //This allows us to shuffle the moveset also to make it less predictable
    //Each ability has a condition check for example if the boss is in melee range only then he will use melee attack
    //Weavere also has a stamina cost as a way to control pacing (a concept i took from ultrakill). If he get exhausted he will stop spaming abilites for awhile
    //The weaver can have multiple ability decks such as one for 50> hp and 50< hp. 

    List<AbilitySequence> AbilityDeckStart = new List<AbilitySequence>();

    //current set of abilities
    List<AbilitySequence> currentAbilityDeck = new List<AbilitySequence>(); 
    


    public enum BossState
    {
        Spawning,
        Idle,
        Walking,
        Targeting,
        SelectAbility,
        AbilityCarryOut,
        Dead

    }


    protected override void  awake()
    {
        //lets create our own ability deck :D




        AbilitySequence[] abilitySequences = {
            //new MeleeAttack(this),
           //new StationaryGroundSlam(this),
           //  new StationaryGroundSlam(this),
           new MissileBarrage(this)
        };

        AbilityDeckStart.AddRange(abilitySequences);
    }

    protected override void init()
    {
        base.enemyStats = getScript<EnemyStats>();
        player = GameObject.FindWithTag("Player");
        playerHead = GameObject.FindWithTag("PlayerHead");
        navMeshAgent.setAutomateNavMeshOfflinksState(false);
        bossStats = getScript<EnemyStats>();
        player = GameObject.FindWithTag("Player");
        navMeshAgent.setIsUpdateRotation(false);
        halfExtent = new Vector3(gameObject.transform.scale.x, gameObject.transform.scale.y, gameObject.transform.scale.z);

        bossUI = GameObject.FindWithTag("Game UI Manager")?.getScript<BossUI>();

        currentStamina = maxStamina;

        //Create an ability deck
        currentAbilityDeck = new List<AbilitySequence>(AbilityDeckStart);

        BossState currentState = BossState.Spawning;

        maxHealth = enemyStats.health;

        //-------------Additional Parameters ----------------------------
        blackboard.SetValue("MeleeRotationSpeed", 200f);
        blackboard.SetValue("PlayerLeftAngle", 10f);


    }

    // This function is invoked every update.
    protected override void update()
    {
        FlushDamageEnemy();
        //Debug.Log(bossStats.health);

        //make the boss look at the player

        Debug.Log("Current State: " + currentState.ToString());

    
        switch (currentState)
        {
            case BossState.Spawning:
                {
                    cooldowntimeElapsed += Time.V_DeltaTime();

                    if (cooldowntimeElapsed > spawningDuration)
                    { 
                        cooldowntimeElapsed = abilityCoolDownTime;
                        currentState = BossState.Idle;
                    }

                }
                break;
            case BossState.Idle:
                {

                    cooldowntimeElapsed += Time.V_DeltaTime();
                    if (cooldowntimeElapsed > abilityCoolDownTime)
                    {
                        if (currentAbilityDeck.Count() == 0 || currentStamina <= 0)
                        {
                            currentAbilityDeck = new List<AbilitySequence>(AbilityDeckStart); //referesh list
                            cooldowntimeElapsed = 0;
                            currentStamina = maxStamina;
                            currentState = BossState.SelectAbility;
                            //Debug.Log("Select Ability");

                        }
                        else
                        { 
                            currentState = BossState.SelectAbility;
                        }


                    }
                    else
                    {
                        //choose to rotate or choose to start walking 

                        //prioritise walking over turning cause turning also walks
                        if (!isWithinIdleTolerance())
                        {
                            currentState = BossState.Walking;
                            SetWalking();
                        }
                        else
                        if (!isWithinTargetingTolerance())
                        {
                            currentState = BossState.Targeting;
                            animator.PlayAnimation("Boss_Run");

                        }

                    }


                }
                break;
            case BossState.Walking:
                {
                    cooldowntimeElapsed += Time.V_DeltaTime();

                    //check if agent is near player
                    Vector3 toPlayer = gameObject.transform.position - player.transform.position;


                    //start walking menacingly towards player
                    if (toPlayer.Length() > stopDistance)
                    {
                        RotateToPlayer();
                        Vector3 playerposMod = player.transform.position;
                        playerposMod.y = + 5;
                        Vector3? playerPosition  = NavigationAPI.SampleNavMeshPosition("Boss", playerposMod, new Vector3(100f,50f,100f));

                        if (playerPosition != null)
                        {

                            NavigationAPI.setDestination(gameObject, playerPosition);
                        }
                        else
                        {
                            Debug.Log("Unable to set position");
                        }


                    }
                    else //within rangle go back to idle
                    {

                        NavigationAPI.stopAgent(gameObject);
                        currentState = BossState.Idle;
                        animator.PlayAnimation("Boss_Idle");

                    }

                    if (cooldowntimeElapsed >= abilityCoolDownTime)
                    {
                        currentState = BossState.Idle;

                        StopWalking();
                    }




                }
                break;
            case BossState.Targeting:
                {
                    cooldowntimeElapsed += Time.V_DeltaTime();


                    //Rotate to player

                    Vector3 direction = player.transform.position - gameObject.transform.position;
                    direction.y = 0;

                    //rotation handling
                    //above or below the player
                    if (direction.Length() > 1f)
                    {
                        direction.Normalize();
                        Quaternion targetRotation = Quaternion.LookRotation(direction);

                        float angleRemaining = Quaternion.Angle(gameObject.transform.rotation, targetRotation);

                        if (angleRemaining > lookAngle)
                        {
                            Debug.Log("Current Angle: " + angleRemaining);
                            //if (isIdleAnimation == true)
                            //{
                            //    animator.PlayAnimation("Boss_Run");
                            //    isIdleAnimation = false;
                            //}
                            gameObject.transform.rotation = Quaternion.RotateTowards(gameObject.transform.rotation, targetRotation, rotationSpeed * Time.V_DeltaTime());
                        }
                        else
                        {
                            currentState = BossState.Idle;
                            animator.PlayAnimation("Boss_Idle");

                        }
                    }



                    //Start ability sequence when ability is up
                    if (cooldowntimeElapsed >= abilityCoolDownTime)
                    {
                        currentState = BossState.Idle;
                        StopWalking();
                    }

                    //if player exits not walking range walk towards player
                    if (!isWithinIdleTolerance())
                    {
                        SetWalking();
                        currentState = BossState.Walking;

                    }
                }
                    break;
            case BossState.SelectAbility:
                {
                    //Shuffle current deck and pick a squence. Then carry it out.
                    if (currentAbilityDeck.Count() > 0 && currentStamina > 0 )
                    {
                        terminateExecution = false;

                        //shuffle deck
                        currentAbilityDeck.Shuffle();
                        //Debug.Log("Shuffle");
                        //System.Random.Shared.Shuffle<AbilitySequence>(currentAbilityDeck);
                        bool noAbilityFound = true;

                        for(int i = 0; i < currentAbilityDeck.Count(); i++)
                        {
                            if (currentAbilityDeck[i].CheckConditions())
                            {
                                //reinitialise mixup
                                currentAbilityDeck[i].ApplyCost();
                                currentState = BossState.AbilityCarryOut;
                                abilitytimeElapsed = 0;
                                abilityIndexer = i;
                                sequenceIndexer = 0;
                                noAbilityFound = false;
                                break;
                            }
                        }

                        if (noAbilityFound == true)
                        {
                            Debug.Log("No ability found");
                            //no ability found go back to idle
                            currentState = BossState.Idle;
                        }

                    }
                    else
                    { 
                        currentState = BossState.Idle;
                    }
                }
                break;
            case BossState.AbilityCarryOut:
                {
                    abilitytimeElapsed += Time.V_DeltaTime();
                    if (terminateExecution || currentAbilityDeck[abilityIndexer].sequence.Count() == sequenceIndexer)
                    {
                        Debug.Log("ability cancelled");
                        currentAbilityDeck.RemoveAt(abilityIndexer);
                        currentState = BossState.SelectAbility;
                        return;
                    }

                    //Debug.Log("current squence name: " + currentAbilityDeck[abilityIndexer].sequence[sequenceIndexer].ToString());
                    //carry out the ability sequence
                    currentAbilityDeck[abilityIndexer].sequence[sequenceIndexer]();

                }
                break;
            case BossState.Dead:
                { 
                
                }
                break;
        }

    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    { 
    }


    public override bool IsEngagedInBattle()
    {
        return true;
    }

    void Idle()
    { 
        currentState = BossState.Walking;
    
    }


    //Rotate Whenever
    public void RotateToPlayer()
    {
        Vector3 direction = player.transform.position - gameObject.transform.position;


        direction.y = 0;

        //above or below the player
        if (direction.Length() > 1f)
        {
            direction.Normalize();
            Quaternion targetRotation = Quaternion.LookRotation(direction);

            float angleRemaining = Quaternion.Angle(gameObject.transform.rotation, targetRotation);

            if (angleRemaining > lookAngle)
            {
                gameObject.transform.rotation = Quaternion.RotateTowards(gameObject.transform.rotation, targetRotation, rotationSpeed * Time.V_DeltaTime());
            }
        }
    }

    public bool isWithinTargetingTolerance()
    {

        Vector3 direction = player.transform.position - gameObject.transform.position;
        direction.y = 0;

        //above or below the player
        if (direction.Length() > 1f)
        {
            direction.Normalize();
            Quaternion targetRotation = Quaternion.LookRotation(direction);

            float angleRemaining = Quaternion.Angle(gameObject.transform.rotation, targetRotation);

            if (angleRemaining <= lookAngle + startLookTolerance)
            {
                return true;
            }


        }

        return false;
    }

    public bool isWithinIdleTolerance()
    {

        Vector3 direction = player.transform.position - gameObject.transform.position;
        direction.y = 0;

        //above or below the player
        if (direction.Length() <= stopDistance + startDistanceTolerance )
        {
            return true;
        }

        return false;
    }

    public void SetWalking()
    {
        animator.PlayAnimation("Boss_Run");
        Vector3? navemshPoint = NavigationAPI.SampleNavMeshPosition("Boss", gameObject.transform.position, new Vector3(1f, 40f, 1f));

        if (navemshPoint == null)
        {
            Debug.LogError("Boss cannot find Navmesh point");
            return;
        }

        navMeshAgent.Warp(navemshPoint);
        navMeshAgent.enable = true;
    }

    public void StopWalking()
    {
        //animator.PlayAnimation("Boss_Run");
        //Vector3? navemshPoint = NavigationAPI.SampleNavMeshPosition("Boss", gameObject.transform.position, new Vector3(1f, 40f, 1f));

        //if (navemshPoint == null)
        //{
        //    Debug.LogError("Boss cannot find Navmesh point");
        //    return;
        //}

        //navMeshAgent.Warp(navemshPoint);
        animator.PlayAnimation("Boss_Idle");
        navMeshAgent.enable = false;
    }



    /***********************************************************
    Weaver Actions
    ***********************************************************/
    public void StarStationaryJumpKinematic()
    {
        AdvanceToNextSequence();

        abilitytimeElapsed = 0;
        navMeshAgent.enable = false;

        blackboard.SetValue("Jump Height", 20.0f);
        blackboard.SetValue("Jump Time",   1.0f);
        blackboard.SetValue("InitialPosition", gameObject.transform.position);
        blackboard.SetValue("EndPosition", gameObject.transform.position);

    }

    public void JumpingKinematic()
    {
        blackboard.TryGetValue("Jump Height", out float jumpHeight);
        blackboard.TryGetValue("InitialPosition", out Vector3 initialPosition);
        blackboard.TryGetValue("EndPosition", out Vector3 endPosition);
        blackboard.TryGetValue("Jump Time", out float jumpTime);

        if (abilitytimeElapsed < jumpTime)
        {
            float t = abilitytimeElapsed / jumpTime;

            Vector3 currentPos = Vector3.Lerp(initialPosition, endPosition, t);


            float heightOffset = jumpHeight * 4 * t * (1 - t);

            currentPos.y += heightOffset;

            gameObject.transform.position = currentPos;
        }
        else
        {
            AdvanceToNextSequence();
        }
    }


    public void TriggerJumpAnimation()
    {
        animator.PlayAnimation("Boss_Jump");
        AdvanceToNextSequence();
    }


    public void CreateShockWave()
    {
        //Debug.Log("Shockwave");
        AdvanceToNextSequence();
        Instantiate(shockwavePrefab, gameObject.transform.position, gameObject.transform.rotation);

        Instantiate(pushFieldPrefab, gameObject.transform.position, gameObject.transform.rotation);

    }

    public void TriggerMissileAnimation()
    {
        animator.PlayAnimation("Boss_Missile");
        AdvanceToNextSequence();
    }

    public void FireMissileCombination()
    { 
        //launch interval
        float launchInterval = 0.0f;

       // int count = launchInterval / abilitytimeElapsed;

        if (abilitytimeElapsed > launchInterval)
        {
            //Launch from all three side

            //Top
            GameObject topMissile = Instantiate(missilePrefab, mainLauncher.transform.position, mainLauncher.transform.rotation);

            //Left
            GameObject leftMissile = Instantiate(missilePrefab, sideLauncher[0].transform.position, sideLauncher[0].transform.rotation);

            GameObject rightMissile = Instantiate(missilePrefab, sideLauncher[1].transform.position, sideLauncher[1].transform.rotation);

            topMissile.getScript<BossHomingMissile>().InitialiseMissileSetting(mainLauncher.transform.front);
            leftMissile.getScript<BossHomingMissile>().InitialiseMissileSetting(sideLauncher[0].transform.front);
            rightMissile.getScript<BossHomingMissile>().InitialiseMissileSetting(sideLauncher[1].transform.front);

            AnimationSpeedAdjustment(0.8f);
            AdvanceToNextSequence();
        }






    }

    public void FireMissileMain()
    {
        //launch interval
        float launchInterval = 0.0f;
        if (abilitytimeElapsed > launchInterval)
        {
            //Top
            GameObject topMissile = Instantiate(missilePrefab, mainLauncher.transform.position, Quaternion.Identity());
            topMissile.getScript<BossHomingMissile>().InitialiseMissileSetting(mainLauncher.transform.front);
            AdvanceToNextSequence();
        }
    }

    public void FannedMeleeAttack()
    {

        float angle = 60;
        int count = 3;

        float angleStep = (count > 1) ? angle / (count - 1) : 0;
        float minimalAngle = -(angle / 2.0f);


        for (int i = 0; i < count; i++)
        {

            float currentAngle = minimalAngle + (angleStep * i);

            Quaternion offset = Quaternion.AngleAxis(Mathf.Deg2Rad * currentAngle, meleeAttackPosition.transform.up);

            Quaternion finalRotation =  Quaternion.LookRotation(-meleeAttackPosition.transform.front) * offset ;

            GameObject meleeWave = Instantiate(meleeWavePrefab, meleeAttackPosition.transform.position, Quaternion.Identity());

            meleeWave.transform.rotation = finalRotation;
        }

        AdvanceToNextSequence();
    }



    public void TriggerMeleeAttackAnimation()
    {
        
        animator.PlayAnimation("Boss_Attack");
        AdvanceToNextSequence();
    }


    public void ReturnToIdle()
    {
        animator.speedMultiplier = 1.0f;
        animator.PlayAnimation("Boss_Idle");
    }

    //return to idle without animations controller
    public void ForcedReturnToIdle()
    {
        AdvanceToNextSequence();
        animator.speedMultiplier = 1.0f;
        animator.PlayAnimation("Boss_Idle");
    }


    public void AwaitAnimation()
    { 
        //Do nothing until animation advances this state
    
    }

    //Use with a lambda to delay action
    public void DelayedSequence( float delayTime)
    { 

        if (abilitytimeElapsed > delayTime)
        {
            AdvanceToNextSequence();
        }
    }

    public void AnimationSpeedAdjustmentGoNext(float value)
    {
        animator.speedMultiplier = value;
        AdvanceToNextSequence();
    }

    public void AnimationSpeedAdjustment(float value)
    {
        animator.speedMultiplier = value;
    }


    public void RestoreDefaultSettings()
    {
        navMeshAgent.enable = true;
        Vector3? position  = NavigationAPI.SampleNavMeshPosition("Boss", gameObject.transform.position, halfExtent);

        if (position != null)
        {
            gameObject.transform.position = position.Value;
            navMeshAgent.Warp(gameObject.transform.position);
        }
    }

    public void RotateToPlayerFully()
    {
        Vector3 direction = player.transform.position - gameObject.transform.position;

       direction.y = 0;

        //above or below the player
        if (direction.Length() > 1f)
        {
            direction.Normalize();
            Quaternion targetRotation = Quaternion.LookRotation(direction);

            float angleRemaining = Quaternion.Angle(gameObject.transform.rotation, targetRotation);
            //Debug.Log( (float) (rotationSpeed * Time.V_DeltaTime()));

            if (angleRemaining > lookAngle)
            {
                gameObject.transform.rotation = Quaternion.RotateTowards(gameObject.transform.rotation,targetRotation,rotationSpeed * Time.V_DeltaTime());
            }
            else
            {
                sequenceIndexer++;

            }
        }
        else {  sequenceIndexer++; }

        

    }

    //due to animation we need to rotate to player left a bit
    public void RotateToPlayerMelee()
    {
        blackboard.TryGetValue("MeleeRotationSpeed", out float rotationBoost);
        blackboard.TryGetValue("PlayerLeftAngle", out float playerLeftAngle);
        Vector3 currenplayerPos = player.transform.position;
        currenplayerPos += (-player.transform.right * playerLeftAngle);
        Vector3 direction = currenplayerPos - gameObject.transform.position;

        direction.y = 0;

        //above or below the player
        if (direction.Length() > 1f)
        {
            direction.Normalize();
            Quaternion targetRotation = Quaternion.LookRotation(direction);

            float angleRemaining = Quaternion.Angle(gameObject.transform.rotation, targetRotation);
            //Debug.Log( (float) (rotationSpeed * Time.V_DeltaTime()));

            if (angleRemaining > lookAngle)
            {
                gameObject.transform.rotation = Quaternion.RotateTowards(gameObject.transform.rotation, targetRotation, (rotationSpeed+ rotationBoost) * Time.V_DeltaTime());
            }
            else
            {
                sequenceIndexer++;

            }
        }
        else { sequenceIndexer++; }



    }


    /******************End of Weaver Action*******************/


    /***********************************************************
    Ability List (Create our Mix Ups in here :D)
    ***********************************************************/
    public class StationaryGroundSlam : AbilitySequence
    {

        public StationaryGroundSlam(Boss boss) : base(boss)
        {
            this.boss = boss;

            //sequence.Add(boss.RotateToPlayerFully);
            sequence.Add(boss.TriggerJumpAnimation);
            sequence.Add(boss.AwaitAnimation); //jump has two triggers
            sequence.Add(boss.AwaitAnimation); //jump has two triggers
            sequence.Add(() => { boss.AnimationSpeedAdjustment(5.0f); });
            sequence.Add(boss.CreateShockWave);
            sequence.Add(() => { boss.DelayedSequence(0.5f); });
            sequence.Add(() => { boss.DelayedSequence(2.0f); });
        }


        public override bool CheckConditions()
        {
            if (boss != null && boss.currentStamina > 1)
            {
                //TBH can apply cost here lmao but i want to keep it clean
                return true;
            }
            return false;
        }
        public override void ApplyCost()
        {
            boss.currentStamina -= 1;
        }
    }


    public class MissileBarrage : AbilitySequence
    {

        public MissileBarrage(Boss boss) : base(boss)
        {
            this.boss = boss;
            sequence.Add(boss.TriggerMissileAnimation);
            sequence.Add(boss.AwaitAnimation); //await for missile
            sequence.Add(boss.AwaitAnimation); //await to skip opening animation
            sequence.Add(boss.FireMissileCombination);
            sequence.Add(boss.AwaitAnimation);
            sequence.Add(() => { boss.AnimationSpeedAdjustment(105.0f); boss.TriggerMissileAnimation();  } ); //retrigger animation
            sequence.Add(boss.AwaitAnimation);
            //sequence.Add(() => { boss.AnimationSpeedAdjustment(52.0f); }); //sync to animation //skip opening squence
            //sequence.Add(() => { boss.AnimationSpeedAdjustment(1.0f); });
            sequence.Add(boss.FireMissileCombination);
            sequence.Add(boss.AwaitAnimation);
            sequence.Add(boss.ForcedReturnToIdle);
            sequence.Add(() => { boss.DelayedSequence(2.0f); }); //quick way to delay action
        }


        public override bool CheckConditions()
        {
            if (boss != null && boss.currentStamina > 2)
            {
               
                return true;
            }
            return false;
        }
        public override void ApplyCost()
        {
            boss.currentStamina -= 2;
        }
    }

    public class MeleeAttack : AbilitySequence
    {
        


        public MeleeAttack(Boss boss) : base(boss)
        {
            this.boss = boss;

            sequence.Add(boss.RotateToPlayerMelee);
            sequence.Add(boss.TriggerMeleeAttackAnimation);
            sequence.Add(boss.AwaitAnimation);
            sequence.Add(boss.FannedMeleeAttack);
            sequence.Add(() => { boss.AnimationSpeedAdjustmentGoNext(1.5f); });
            sequence.Add( () => { boss.DelayedSequence(0.5f); }); //quick way to delay action
            //sequence.Add(() => { boss.DelayedSequence(2.0f); });
        }


        public override bool CheckConditions()
        {
            Vector3 direction = boss.meleeAttackPosition.transform.position  - boss.player.transform.position;


            if (boss != null && boss.currentStamina > 1 && direction.Length() < 300)
            {

                return true;
            }
            return false;
        }
        public override void ApplyCost()
        {
            boss.currentStamina -= 1;
        }
    }


    /***************End of Ability List *****************/



    //Helper function to advance to the next sequence in the ability
    public void AdvanceToNextSequence()
    {
        //Debug.Log("Advance to Next Squence");
        sequenceIndexer++;
        abilitytimeElapsed = 0;
    
    }


    public override void TakeDamage(float damage, Enemy.EnemydamageType damageType, string colliderTag)
    {
        if (damageType == Enemy.EnemydamageType.WeaponShot)
        {


            if (colliderTag == "Enemy_ArmouredSpot")
            {
                damage *= bossStats.enemyArmouredMultiplier;

            }
            if (colliderTag == "Enemy_WeakSpot")
            {
                damage *= bossStats.enemyWeakSpotMultiplier;

            }

            accumulatedDamageInstance += damage;


        }

        if (damageType == Enemy.EnemydamageType.ThrownWeapon)
        {
            if (bossStats.health <= bossStats.enemyExecuteThreshold)
            {
                //Explode();
                ////animator.PlayAnimation("Grunt Death");
                ////NavigationAPI.stopAgent(gameObject);
                ////rigidbody.enable = false;
                //bossStats = bos;
                //if (gameObject != null)
                //    Destroy(gameObject);

            }
            else
            {
                accumulatedDamageInstance += damage;
            }

        }

        if (damageType == Enemy.EnemydamageType.Ultimate)
        {

            accumulatedDamageInstance += damage;

        }

    }

    void FlushDamageEnemy()
    {
        if (accumulatedDamageInstance > 0)
        {
            SpawnIchorFrame();
            bossStats.health -= accumulatedDamageInstance;

            bossUI?.SetBossHealth(bossStats.health + accumulatedDamageInstance, bossStats.health, maxHealth);

            if (bossStats.health <= 0)
            {
                if (currentState != BossState.Dead)
                {
                    currentState = BossState.Dead;

                    //audioComponent.PlayRandomSound(deathSFX);
                    //animator.PlayAnimation("Grunt Death");
                    DisablePhysicalInteraction();

                    animator.PlayAnimation("Boss_Death");
                }
            }
            else
            {
                TriggerRecentlyDamageCountdown();
                if (currentState != BossState.Dead && !WasRecentlyDamaged())
                {

                    //renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 0f, 0f));
                    //renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 0f, 0f));
                    Invoke(() =>
                    {
                        //renderer.setMaterialVector3(0, "colorTint", new Vector3(1f, 1f, 1f));
                        //renderer.setMaterialVector3(1, "colorTint", new Vector3(1f, 1f, 1f));
                    }, bossStats.hurtDuration); //bug here is this object dies this frame
                }
            }
            accumulatedDamageInstance = 0;
        }
    }

    public override void SetSpawningDuration(float seconds)
    {
        
    }





}

public abstract class AbilitySequence
{
    public Boss boss;
   // public int sequenceIndex;
    public List<Action> sequence = new List<Action>();

    // A shared constructor to set the boss
    public AbilitySequence(Boss bossInstance)
    {
        this.boss = bossInstance;
    }

    public abstract bool CheckConditions();
    public abstract void ApplyCost();
}



public static class ListExtensions
{
    public static void Shuffle<T>(this IList<T> list)
    {
        int n = list.Count;
        while (n > 1)
        {
            n--;
            int k = System.Random.Shared.Next(n + 1);
            T value = list[k];
            list[k] = list[n];
            list[n] = value;
        }
    }
}

public class Blackboard
{
    private readonly Dictionary<string, object> entries = new();

    public void SetValue<T>(string key, T value)
    {
        entries[key] = value;
    }

    public bool TryGetValue<T>(string key, out T value)
    {

        if (entries.TryGetValue(key, out var obj) && obj is T typedValue)
        {
            value = typedValue;
            return true;
        }

        Debug.LogError("Blackboard does not contain key: " + key);
        value = default;
        return false;
    }
}

