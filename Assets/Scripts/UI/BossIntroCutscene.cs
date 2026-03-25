// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using System.Runtime.CompilerServices;
using ScriptingAPI;

class BossIntroCutscene : Script
{
    // =========================================================
    // References
    [SerializableField] private Image_ darkOverlay;
    [SerializableField] private CameraComponent_ cutsceneCameraOne;
    [SerializableField] private CameraComponent_ cutsceneCameraTwo;
    [SerializableField] private GameObject animatedPlayer;
    [SerializableField] private Transform_ animatedBossStartingLocation;
    [SerializableField] private Transform_ animatedBossLandingLocation;
    [SerializableField] private Transform_ playerStartPosition;
    [SerializableField] private GameObject arenaLight;

    [SerializableField] private Canvas_ playerUI;
    [SerializableField] private Image_ gunUltimateGlow;
    [SerializableField] private GameObject cutsceneUI;
    [SerializableField] private Canvas_ bossUI;

    [SerializableField] private Transform_ cutsceneTopBar;
    [SerializableField] private Transform_ cutsceneBottomBar;
    [SerializableField] private MeshRenderer_ innerWallCollider;
    [SerializableField] private SimpleBGMManager simpleBGMManager;

    [SerializableField] private Boss boss;
    [SerializableField] private Animator_ bossAnimator;

    // =========================================================
    // Initial fade out
    [SerializableField] private float initialCutsceneDelay = 1f;
    [SerializableField] private float initialFadeOutDuration = 2f;

    // =========================================================
    // Player walk up duration..
    [SerializableField] private float playerWalkDuration = 3.5f;

    // =========================================================
    // Change camera..
    [SerializableField] private float durationBeforeCameraChange = 6f;

    // =========================================================
    // Dialogue
    [SerializableField] private Audio bossIntroVoiceLine;
    [SerializableField] private List<string> bossVoiceLines;
    [SerializableField] private List<float> bossVoiceLineTime;

    // =========================================================
    // Boss drop..
    [SerializableField] private float bossDropDuration = 0.6f;
    [SerializableField] private float delayBeforeVoiceLine = 2f;

    // =========================================================
    // Camera pans back..
    [SerializableField] private float delayBeforeCameraPanback = 11f;
    [SerializableField] private float delayBeforeArenaLightTurnsOn = 12f;
    [SerializableField] private float delayBeforeCutsceneBar = 14f;
    [SerializableField] private float delayBeforeInnerWallAppearance = 14f;

    [SerializableField] private float cameraPanbackLerpDuration = 5f;
    [SerializableField] private float cameraPanbackLerpPower = 0.45f;

    [SerializableField] private float cutsceneBarLerpDuration = 1f;
    [SerializableField] private float cutsceneBarLerpPower = 0.4f;

    [SerializableField] private float innerWallLerpDuration = 1.4f;
    [SerializableField] private float innerWallLerpPower = 1f;
    [SerializableField] private int innerWallBlinkOccurences = 4;

    [SerializableField] private float arenalightLerpDuration = 1f;
    [SerializableField] private float arenalightLerpPower = 0.6f;

    [SerializableField] private List<Audio> playerWalkingSteps;
    [SerializableField] private float footstepInterval = 0.2f;

    [SerializableField] private Audio bossCrashLandingSound;

    // =========================================================
    // Runtime variables..
    private GameUIManager gameUIManager;
    private PlayerController_V2 player;
    private PlayerWeaponController playerWeaponController;
    private PlayerRotateController playerRotateController;
    private CameraComponent_ playerCamera;
    private GameObject sniper;
    private AudioComponent_ audioComponent;

    // --- Initial fade out ---
    private bool isAnimatingInitialFadeOut = false;
    private float timeElapsed = 0f;

    // --- Player walking ---
    private bool toStopPlayerFromWalking = false;
    private bool hasPlayerStoppedWalking = false;

    // --- Boss locations ---
    private bool isAnimatingBossDrop = false;
    private Vector3 animatedBossStartLocation;
    private Vector3 animatedBossEndLocation;

    // --- Camera pan ---
    private bool isAnimatingCameraPanback = false;
    private Vector3 cutsceneInitialPosition;
    private Vector3 cutsceneFinalPosition;
    private Quaternion cutsceneInitialRotation;
    private Quaternion cutsceneFinalRotation;

    // --- Cutscene bars ---
    private bool isAnimatingCutsceneBar = false;
    private float cutsceneBarTimeElapsed = 0f;

    private Vector3 cutsceneTopBarInitialPosition;
    private Vector3 cutsceneTopBarFinalPosition;

    private Vector3 cutsceneBottomBarInitialPosition;
    private Vector3 cutsceneBottomBarFinalPosition;

    // --- Inner wall ---
    private bool isAnimatingInnerWall = false;
    private float innerWallTimeElapsed = 0f;

    // --- Inner wall ---
    private bool isAnimatingLight = false;
    private float animatingLightTimeElapsed;

    private float arenaLightIntensity = 0f;

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
        audioComponent = getComponent<AudioComponent_>();

        gameUIManager = GameObject.FindWithTag("Game UI Manager")?.getScript<GameUIManager>();
        player = GameObject.FindWithTag("Player")?.getScript<PlayerController_V2>();
        playerWeaponController = player.getScript<PlayerWeaponController>();
        playerRotateController = GameObject.FindWithTag("PlayerHead")?.getScript<PlayerRotateController>();
        playerCamera = GameObject.FindWithTag("PlayerCamera").getComponent<CameraComponent_>();

        sniper = GameObject.FindWithTag("Sniper");

        animatedBossStartLocation = animatedBossStartingLocation.position;
        animatedBossEndLocation = animatedBossLandingLocation.position;

        cutsceneTopBarInitialPosition = cutsceneTopBar.position;
        cutsceneTopBarFinalPosition = new Vector3(cutsceneTopBar.position.x, cutsceneTopBar.position.y + cutsceneTopBar.scale.y, cutsceneTopBar.position.z);

        cutsceneBottomBarInitialPosition = cutsceneBottomBar.position;
        cutsceneBottomBarFinalPosition = new Vector3(cutsceneBottomBar.position.x, cutsceneBottomBar.position.y - cutsceneTopBar.scale.y, cutsceneBottomBar.position.z);

        foreach (GameObject child in arenaLight.GetChildren())
        {
            Light_ light = child.getComponent<Light_>();

            if (light != null)
            {
                arenaLightIntensity = light.intensity;
                break;
            }
        }
        
        // in the future, we control when to start the cutscene, not in init.
        BeginCutscene();
    }

    public void BeginCutscene()
    {
        playerUI.gameObject.SetActive(false);

        sniper.SetActive(false);
        player.movementIsEnabled = false;
        playerWeaponController.DisableShooting();
        playerWeaponController.DisableWeaponArm();
        playerRotateController.rotationIsEnabled = false;

        animatedPlayer.getScript<Translation>().move();
        boss.getComponent<NavMeshAgent_>().enable = false;

        bossUI.gameObject.SetActive(false);

        Invoke(() =>
        {
            cutsceneUI.SetActive(true);

            playerCamera.camStatus = false;
            cutsceneCameraOne.camStatus = true;
            cutsceneCameraTwo.camStatus = false;

            isAnimatingInitialFadeOut = true;
            cutsceneCameraOne.gameObject.getComponent<Sequence_>().play();

            player.gameObject.transform.position = playerStartPosition.position;
            playerRotateController.gameObject.transform.rotation = Quaternion.LookRotation(playerStartPosition.front);

            RecursiveFootstep();

            Invoke(() =>
            {
                toStopPlayerFromWalking = true;
            }, playerWalkDuration);

            Invoke(() =>
            {
                cutsceneCameraOne.camStatus = false;
                cutsceneCameraTwo.camStatus = true;

                cutsceneCameraTwo.gameObject.getComponent<Sequence_>().play();
                boss.getComponent<Sequence_>().play();
                bossAnimator.PlayAnimation("Boss_Run");
            }, durationBeforeCameraChange);

        }, initialCutsceneDelay);
    }

    private void RecursiveFootstep()
    {
        if(hasPlayerStoppedWalking)
        {
            return;
        }

        Invoke(() =>
        {
            audioComponent.PlayRandomSound(playerWalkingSteps);
            RecursiveFootstep();
        }, footstepInterval);
    }

    // This function is invoked every update.
    protected override void update()
    {
        AnimatingInitialFadeOut();
        AnimatingBossDrop();
        AnimatingCameraPanback();
        AnimatingCutsceneBars();
        AnimatingInnerWall();
        AnimatingLight();
    }

    private void AnimatingInitialFadeOut()
    {
        if (!isAnimatingInitialFadeOut)
        {
            return;
        }
        
        float interval = timeElapsed / initialFadeOutDuration;
        darkOverlay.colorTint = new ColorAlpha(0f, 0f, 0f, Mathf.Pow(1f - interval, 1f / 2.2f));

        timeElapsed += Time.V_DeltaTime();

        if (timeElapsed > initialFadeOutDuration)
        {
            isAnimatingInitialFadeOut = false;
            darkOverlay.colorTint = new ColorAlpha(0f, 0f, 0f, 0f);
        }
    }

    private void AnimatingBossDrop()
    {
        if (!isAnimatingBossDrop)
        {
            return;
        }

        float interval = timeElapsed / bossDropDuration;
        boss.gameObject.transform.position = Vector3.Lerp(animatedBossStartLocation, animatedBossEndLocation, interval);

        timeElapsed += Time.V_DeltaTime();

        if (timeElapsed > bossDropDuration)
        {
            animatedPlayer.gameObject.SetActive(false);
            isAnimatingBossDrop = false;
            boss.gameObject.transform.position = animatedBossLandingLocation.position;
            animatedBossLandingLocation.gameObject.getComponent<ParticleEmitter_>().emit();

            CameraAPI.shakeCamera(0.5f, 10f);
            audioComponent.PlaySound(bossCrashLandingSound);

            Invoke(() =>
            {
                gameUIManager.getScript<VoiceoverScript>().TriggerVoiceOverSequence("Weaver", bossVoiceLines, bossIntroVoiceLine, bossVoiceLineTime, false);
            }, delayBeforeVoiceLine);

            Invoke(() =>
            {
                isAnimatingCameraPanback = true;
                timeElapsed = 0f;
                sniper.SetActive(true);

                cutsceneInitialPosition = cutsceneCameraTwo.gameObject.transform.position;
                cutsceneFinalPosition = playerCamera.gameObject.transform.position;

                cutsceneInitialRotation = cutsceneCameraTwo.gameObject.transform.rotation;
                cutsceneFinalRotation = playerCamera.gameObject.transform.rotation;
            }, delayBeforeCameraPanback);

            Invoke(() =>
            {
                arenaLight.SetActive(true);
                isAnimatingLight = true;
                animatingLightTimeElapsed = 0f;

                foreach (GameObject child in arenaLight.GetChildren())
                {
                    Light_ light = child.getComponent<Light_>();

                    if (light != null)
                    {
                        light.intensity = 0;
                    }
                }

            }, delayBeforeArenaLightTurnsOn);

            Invoke(() =>
            {
                isAnimatingInnerWall = true;
            }, delayBeforeInnerWallAppearance);

            Invoke(() =>
            {
                isAnimatingCutsceneBar = true;
                cutsceneBarTimeElapsed = 0f;
                playerUI.gameObject.SetActive(true);
                playerUI.alpha = 0f;
                gunUltimateGlow.gameObject.SetActive(false);

                bossUI.gameObject.SetActive(true);
                bossUI.alpha = 0f;
            }, delayBeforeCutsceneBar);
        }
    }

    private void AnimatingCameraPanback()
    {
        if(!isAnimatingCameraPanback)
        {
            return;
        }

        float interval = Mathf.Pow(timeElapsed / cameraPanbackLerpDuration, cameraPanbackLerpPower);
        cutsceneCameraTwo.gameObject.transform.position = Vector3.Lerp(cutsceneInitialPosition, cutsceneFinalPosition, interval);
        cutsceneCameraTwo.gameObject.transform.rotation = Quaternion.Slerp(cutsceneInitialRotation, cutsceneFinalRotation, interval);

        timeElapsed += Time.V_DeltaTime();

        if (timeElapsed > cameraPanbackLerpDuration)
        {
            cutsceneCameraTwo.gameObject.transform.position = cutsceneFinalPosition;
            isAnimatingCameraPanback = false;
        }
    }

    private void AnimatingCutsceneBars()
    {
        if(!isAnimatingCutsceneBar)
        {
            return;
        }

        float interval = Mathf.Pow(cutsceneBarTimeElapsed / cutsceneBarLerpDuration, cutsceneBarLerpPower);

        cutsceneTopBar.position = Vector3.Lerp(cutsceneTopBarInitialPosition, cutsceneTopBarFinalPosition, interval);
        cutsceneBottomBar.position = Vector3.Lerp(cutsceneBottomBarInitialPosition, cutsceneBottomBarFinalPosition, interval);
        playerUI.alpha = interval;
        bossUI.alpha = interval;

        cutsceneBarTimeElapsed += Time.V_DeltaTime();

        if (cutsceneBarTimeElapsed > cutsceneBarLerpDuration)
        {
            playerUI.alpha = 1f;
            bossUI.alpha = 1f;

            cutsceneUI.SetActive(false);
            isAnimatingCutsceneBar = false;

            cutsceneCameraTwo.camStatus = false;
            playerCamera.camStatus = true;
            player.movementIsEnabled = true;
            playerWeaponController.EnableShooting();
            playerWeaponController.EnableWeaponArm();
            playerRotateController.rotationIsEnabled = true;

            boss.getComponent<NavMeshAgent_>().enable = true;
            boss.GoToIdleState(); 

            simpleBGMManager.playBGM();
        }
    }

    private void AnimatingInnerWall()
    {
        if (!isAnimatingInnerWall)
        {
            return;
        }

        float interval = Mathf.Pow(innerWallTimeElapsed / innerWallLerpDuration, innerWallLerpPower);
        innerWallCollider.setMaterialFloat(0, "alpha", interval);

        innerWallTimeElapsed += Time.V_DeltaTime();

        if (innerWallTimeElapsed > innerWallLerpDuration)
        {
            innerWallCollider.setMaterialFloat(0, "alpha", 1);
            isAnimatingInnerWall = false;
            //RecursiveWallBlink();
        }
    }

    int counter = 0;
    bool isOn = true;

    private void RecursiveWallBlink()
    {
        Invoke(() =>
        {
            isOn = !isOn;
            counter++;

            if (isOn) {
                innerWallCollider.setMaterialFloat(0, "alpha", 1);
            }
            else
            {
                innerWallCollider.setMaterialFloat(0, "alpha", 0);
            }
            
            if(counter < innerWallBlinkOccurences)
            {
                RecursiveWallBlink();
            }
        }, Random.Range(0.05f, 0.1f) * counter);
    }

    private void AnimatingLight()
    {
        if (!isAnimatingLight)
        {
            return;
        }

        float interval = Mathf.Pow(animatingLightTimeElapsed / arenalightLerpDuration, arenalightLerpPower);
        animatingLightTimeElapsed += Time.V_DeltaTime();

        foreach (GameObject child in arenaLight.GetChildren())
        {
            child.getComponent<Light_>().intensity = Mathf.Interpolate(0f, arenaLightIntensity, interval, 1f);
        }

        if (animatingLightTimeElapsed > arenalightLerpDuration)
        {
            isAnimatingLight = false;
        }
    }

    // Invoked by animation event.
    public void EndOfPlayerWalk()
    {
        if (!toStopPlayerFromWalking)
        {
            return;
        }

        animatedPlayer.getComponent<Animator_>().speedMultiplier = 0f;
        animatedPlayer.getScript<Translation>().stop();

        hasPlayerStoppedWalking = true;
    }

    // Invoked by animation event.
    public void StartBossDrop()
    {
        boss.getComponent<Sequence_>().pause();
        bossAnimator.PlayAnimation("Boss_Jump");
        bossAnimator.SetFrame(74);
        isAnimatingBossDrop = true;
        timeElapsed = 0f;

        boss.gameObject.transform.position = animatedBossStartingLocation.position;
        boss.gameObject.transform.localRotation = Quaternion.LookRotation(Vector3.Front());
    }

    public void PauseCutsceneCameraTwo()
    {
        cutsceneCameraTwo.gameObject.getComponent<Sequence_>().pause();
    }
}