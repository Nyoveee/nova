        // Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using static GameUIManager;

class GameUIManager : Script
{
    public enum ProgressBarType
    {
        HealthBar,
        DashBar,
        UltimateBar
    }

    // ========================================================================
    // References to game objects..
    [SerializableField]
    private List<Image_> dashFillBars = null;

    [SerializableField]
    private Image_ healthFill = null;

    [SerializableField]
    private Image_ fadeToBlackBackground = null;

    [SerializableField]
    private Image_ healthDamageBuffer = null;

    [SerializableField]
    private Transform_ crossHairUi = null;

    [SerializableField]
    private Image_? gunUltimateFill = null;

    [SerializableField]
    private Image_? gunUltimateGlow = null;

    [SerializableField]
    private GameObject? tutorialUI = null;

    [SerializableField]
    private GameObject? pauseUI = null;

    [SerializableField]
    private GameObject? deathOverlay = null;

    [SerializableField]
    private Image_? deathScreenEffectUi= null;

    [SerializableField]
    private Text_? questText = null;

    [SerializableField]
    private List<Image_> ammoFills;

    [SerializableField]
    private Texture ammoFillTexture;

    [SerializableField]
    private Texture ammoUsedTexture;

    [SerializableField]
    private GameObject playerUI;

    [SerializableField]
    private GameObject missionObjectiveUI;

    // ========================================================================
    // Cross Hair Related Serialized Fields..
    [SerializableField]
    private float crossHairExpandScaleRatio = 2f;

    [SerializableField]
    private float crossHairAnimationDuration = 0.6f;

    [SerializableField]
    private float crossHairLerpPower = 0.7f;

    // ========================================================================
    // Health Change Animation Fields..
    [SerializableField]
    private float healthLossLerpDuration = 0.7f;

    [SerializableField]
    private float healthLossLerpPower = 0.7f;

    [SerializableField]
    private float healthLossBufferTime = 2f;

    [SerializableField]
    private float healthBufferLerpDuration = 0.7f;

    [SerializableField]
    private float healthBufferLerpPower = 0.7f;

    // ========================================================================
    // Health Restoration Animation Fields..
    [SerializableField]
    private float healthGainLerpDuration = 0.7f;

    [SerializableField]
    private float healthGainLerpPower = 0.7f;

    [SerializableField]
    private ColorAlpha healthGainColor;

    // ========================================================================
    // Dash UI..
    [SerializableField]
    private float readyDashBrightness = 1.2f;

    // ========================================================================
    // Gun Ultimate Bar Fill Animation..
    [SerializableField]
    private float ultimateGunFillLerpDuration = 0.7f;

    [SerializableField]
    private float ultimateGunFillLerpPower = 0.7f;

    // ========================================================================
    // Gun Ultimate Bar Glow Vfx..
    [SerializableField]
    private float ultimateBarGlowBrightness = 2f;

    [SerializableField]
    private float ultimateBarGlowBrightnessVariance = 0.5f;

    [SerializableField]
    private float ultimateBarGlowSpeedFactor = 1.0f;

    // ========================================================================
    // Cutscene related stuff..
    [SerializableField]
    private float blackOverlayLerpDuration = 1f;

    [SerializableField]
    private float blackOverlayLerpPower = 0.4f;

    // ========================================================================
    // Death sequence stuff..
    [SerializableField]
    private Texture glassCrackTextureOne;

    [SerializableField]
    private Texture glassCrackTextureTwo;

    [SerializableField]
    private float deathRotationDuration = 0.8f;

    [SerializableField]
    private float deathRotationPower = 1.6f;

    [SerializableField]
    private float totalDeathDuration = 4f;

    [SerializableField]
    private float chromaticAberrationInterval = 0.2f;

    [SerializableField]
    private float blinkingEffectDelay = 0.5f;

    [SerializableField]
    private float blinkingEffectDuration = 2f;

    // ========================================================================
    // ========================================================================
    // Runtime variables

    // ------------------------------------
    // References
    private PlayerController_V2 playerController = null;
    private Dictionary<ProgressBarType, Image_> progressBars = new();
    private DialogueScript dialogueScript = null;

    // ------------------------------------
    // Animating the cross hair when player fires..
    private bool isAnimatingCrossHair = false;
    private float crossHairTimeElasped = 0f;

    private Vector3 initialCrossHairScale;
    private Vector3 expandedCrossHairScale;

    // ------------------------------------
    // Animating health change loss when player takes damage..
    private bool isAnimatingHealthLoss = false;
    private float healthLossTimeElapsed = 0f;

    private Vector2 initialPlayerHealthPercentage;
    private Vector2 finalPlayerHealthPercentage;

    private Vector2 initialHeathLossBufferPercentage;
    private Vector2 finalHeathLossBufferPercentage;

    // this keeps track of the very first initial health loss percentage, when multiple instances of damage is taken..
    private Vector2 lastHeathLossBufferPercentage;

    // Whenever a player first takes damage, we display the loss amount of health..
    // After a certain duration, we lerp this health buffer away..
    private bool isWaitingForHealthRecovery = false;
    private float healthRecoveryCountdown = 0f;

    private bool isAnimatingHealthBuffer = false;
    private float healthBufferTimeElapsed = 0f;

    private bool isDamageBufferActive = false;

    // ------------------------------------
    // Animating health gain
    private bool isAnimatingHealthGain = false;
    private float healthGainTimeElapsed = 0f;

    private ColorAlpha originalHealthGainColor;

    // ------------------------------------
    // Handling dash..
    private ColorAlpha originalDashFillColor = new ColorAlpha(1, 1, 1, 1);
    private ColorAlpha readyDashFillColor = new ColorAlpha(1, 1, 1, 1);

    // ------------------------------------
    // Handling ultimate..
    private bool gunIsGlowing = false;
    private float gunGlowTimeElapsed = 0f;

    private ColorAlpha gunUltimateOriginalColor;

    private bool isAnimatingUltimateFill = false;
    private float ultimateFillTimeElapsed = 0f;

    private Vector2 initialUltimateFillPercentage;
    private Vector2 finalUltimateFillPercentage;

    // ------------------------------------
    private int previousAmmoCount = 12;

    private bool isPaused = false;
    private bool isTutorialPromptActive = false;
    public event Action RestartFromCheckpointButton;

    // ------------------------------------
    // Death sequence..
    private CameraComponent_ camera;

    private bool isAnimatingDeathSequenceRotation = false;
    private float deathSequencetimeElapsed = 0f;

    private Quaternion initialDeathRotation;
    private Quaternion finalDeathRotation;

    private bool isAnimatingBlinkingEffect = false;
    private float blinkingEffectTimeElapsed = 0f;

    private bool playerIsDying = false;

    // ========================================================================
    protected override void init()
    {
        playerController = GameObject.FindWithTag("Player")?.getScript<PlayerController_V2>();
        dialogueScript = getScript<DialogueScript>();
        progressBars[ProgressBarType.HealthBar] = healthFill;
        MapKey(Key.P, PauseHandler, true);

        initialCrossHairScale = crossHairUi.scale;
        expandedCrossHairScale = crossHairUi.scale * crossHairExpandScaleRatio;

        originalHealthGainColor = healthFill.colorTint;

        for (int i = 0; i < dashFillBars.Count; ++i)
        {
            Image_ dashFillBar = dashFillBars[i];
            originalDashFillColor = dashFillBar.colorTint;

            readyDashFillColor = new ColorAlpha(originalDashFillColor.r * readyDashBrightness, originalDashFillColor.b * readyDashBrightness, originalDashFillColor.g * readyDashBrightness, originalDashFillColor.a * readyDashBrightness);
            break;
        }

        gunUltimateOriginalColor = gunUltimateGlow.colorTint;
        camera = GameObject.FindWithTag("PlayerCamera")?.getComponent<CameraComponent_>();

        Restart();
    }

    public void Restart()
    {
        healthFill.textureCoordinatesEnd = new Vector2(1, 1);
        healthDamageBuffer.textureCoordinatesEnd = new Vector2(1, 1);
        initialHeathLossBufferPercentage = new Vector2(0, 1);
        healthDamageBuffer.textureCoordinatesStart = new Vector2(0, 1);
        lastHeathLossBufferPercentage = new Vector2(1, 1);

        isAnimatingCrossHair = false;
        isAnimatingHealthBuffer = false;
        isAnimatingHealthLoss = false;
        crossHairUi.scale = initialCrossHairScale;

        healthFill.colorTint = originalHealthGainColor;

        foreach (Image_ ammoFill in ammoFills)
        {
            ammoFill.SetTexture(ammoFillTexture);
        }

        gunUltimateGlow.gameObject.SetActive(false);
        gunUltimateFill.gameObject.SetActive(true);

        gunUltimateFill.textureCoordinatesEnd = new Vector2(0, 1);

        camera.toRotateSideWays = false;
        camera.ResetUp();
        playerIsDying = false;
        
        isPaused = false;
        
        CameraAPI.LockMouse();

        deathOverlay.gameObject.SetActive(false);
        deathScreenEffectUi.gameObject.SetActive(false);
        
        playerUI.SetActive(true);
        //missionObjectiveUI.SetActive(true);
        
        Systems.Pause = false;
        camera.ResetUp();

        playerController.Reset();
        getScript<VignetteController>().TriggerVignette(0.00f, 0, new Colour(0.0f, 0.0f, 0.0f));
    }

    protected override void update()
    {
        AnimatingCrossHairFire();
        AnimatingHealthLoss();
        AnimatingHealthGain();
        AnimatingHealthLossBufferRecovery();
        AnimatingGunGlow();
        AnimatingUltimateFill();
        AnimatingDeathSequence();
        AnimatingBlinkingEffect();
    }

    /***********************************************************
       Animations..
    ***********************************************************/
    private void AnimatingCrossHairFire()
    {
        if (!isAnimatingCrossHair)
        { 
            return; 
        }

        crossHairUi.scale = Vector3.Lerp(expandedCrossHairScale, initialCrossHairScale, Mathf.Pow(crossHairTimeElasped / crossHairAnimationDuration, crossHairLerpPower));
        crossHairTimeElasped += Time.V_DeltaTime();

        if (crossHairTimeElasped > crossHairAnimationDuration)
        {
            isAnimatingCrossHair = false;
        }
    }

    private void AnimatingHealthLoss()
    {
        // Animating health loss..
        if (!isAnimatingHealthLoss)
        {
            return;
        }

        healthFill.textureCoordinatesEnd = Vector2.Lerp(initialPlayerHealthPercentage, finalPlayerHealthPercentage, Mathf.Pow(healthLossTimeElapsed / healthLossLerpDuration, healthLossLerpPower));
        healthDamageBuffer.textureCoordinatesStart = Vector2.Lerp(initialHeathLossBufferPercentage, finalHeathLossBufferPercentage, Mathf.Pow(healthLossTimeElapsed / healthLossLerpDuration, healthLossLerpPower));

        healthLossTimeElapsed += Time.V_DeltaTime();
        
        if (healthLossTimeElapsed > healthLossLerpDuration)
        {
            isAnimatingHealthLoss = false;
            isWaitingForHealthRecovery = true;
            healthRecoveryCountdown = 0f;
        }
    }

    private void AnimatingHealthGain()
    {
        // Animating health loss..
        if (!isAnimatingHealthGain)
        {
            return;
        }

        float interval = Mathf.Pow(healthGainTimeElapsed / healthGainLerpDuration, healthGainLerpPower);
        healthFill.textureCoordinatesEnd = Vector2.Lerp(initialPlayerHealthPercentage, finalPlayerHealthPercentage, interval);
        healthDamageBuffer.textureCoordinatesStart = Vector2.Lerp(initialHeathLossBufferPercentage, finalHeathLossBufferPercentage, interval);

        healthFill.colorTint = ColorAlpha.Lerp(healthGainColor, originalHealthGainColor, interval);

        healthGainTimeElapsed += Time.V_DeltaTime();

        if (healthGainTimeElapsed > healthGainLerpDuration)
        {
            isAnimatingHealthGain = false;
            isWaitingForHealthRecovery = true;
            healthRecoveryCountdown = 0f;
        }
    }

    private void AnimatingHealthLossBufferRecovery()
    {
        // We wait for a while before recovering the health loss buffer..
        if (isWaitingForHealthRecovery)
        {
            healthRecoveryCountdown += Time.V_DeltaTime();

            if (healthRecoveryCountdown > healthLossBufferTime)
            {
                healthBufferTimeElapsed = 0f;
                isWaitingForHealthRecovery = false;
                isAnimatingHealthBuffer = true;

                initialHeathLossBufferPercentage = new Vector2(1, initialHeathLossBufferPercentage.y);
                finalHeathLossBufferPercentage = new Vector2(1, finalHeathLossBufferPercentage.y);
            }
        }

        if (!isAnimatingHealthBuffer)
        {
            return;
        }

        // Recover health loss buffer..
        healthDamageBuffer.textureCoordinatesEnd = Vector2.Lerp(lastHeathLossBufferPercentage, finalHeathLossBufferPercentage, Mathf.Pow(healthBufferTimeElapsed / healthBufferLerpDuration, healthBufferLerpPower));

        healthBufferTimeElapsed += Time.V_DeltaTime();

        if (healthBufferTimeElapsed > healthBufferLerpDuration)
        {
            isAnimatingHealthBuffer = false;
        }
    }

    private void AnimatingUltimateFill()
    { 
        if(!isAnimatingUltimateFill)
        {
            return;
        }

        float interval = Mathf.Pow(ultimateFillTimeElapsed / ultimateGunFillLerpDuration, ultimateGunFillLerpPower);
        gunUltimateFill.textureCoordinatesEnd = Vector2.Lerp(initialUltimateFillPercentage, finalUltimateFillPercentage, interval);

        ultimateFillTimeElapsed += Time.V_DeltaTime();

        if (ultimateFillTimeElapsed > ultimateGunFillLerpDuration)
        {
            isAnimatingUltimateFill = false;

            // Fully filled.
            if(finalUltimateFillPercentage.x == 1)
            {
                gunIsGlowing = true;
                gunGlowTimeElapsed = 0f;
                gunUltimateGlow.gameObject.SetActive(true);
                gunUltimateFill.gameObject.SetActive(false);
            }
        }
    }
    private void AnimatingGunGlow()
    { 
        if(!gunIsGlowing)
        {
            return;
        }

        gunGlowTimeElapsed += Time.V_DeltaTime() * ultimateBarGlowSpeedFactor;
        gunGlowTimeElapsed = gunGlowTimeElapsed % (Mathf.Deg2Rad * 360f);

        float interval = Mathf.Sin(gunGlowTimeElapsed);
        gunUltimateGlow.colorTint = gunUltimateOriginalColor * (ultimateBarGlowBrightness + interval * ultimateBarGlowBrightnessVariance);
    }

    private void AnimatingDeathSequence()
    {
        if (!isAnimatingDeathSequenceRotation)
        {
            return;
        }

        float interval = Mathf.Pow(deathSequencetimeElapsed / deathRotationDuration, deathRotationPower);
        
        camera.gameObject.transform.rotation = Quaternion.Slerp(initialDeathRotation, finalDeathRotation, interval);

        deathSequencetimeElapsed += Time.V_DeltaTime();

        if (deathSequencetimeElapsed > deathRotationDuration)
        {
            isAnimatingDeathSequenceRotation = false;
            camera.gameObject.transform.rotation = finalDeathRotation;

            CameraAPI.shakeCamera(0.7f, 3f);
            deathScreenEffectUi.SetTexture(glassCrackTextureTwo);
            RendererAPI.blur = true;

            Invoke(() =>
            {
                isAnimatingBlinkingEffect = true;
                blinkingEffectTimeElapsed = 0f;
                RendererAPI.vignetteColor = new Colour(0, 0, 0);

            }, blinkingEffectDelay);
        }
    }

    private void AnimatingBlinkingEffect()
    {
        if (!isAnimatingBlinkingEffect)
        {
            return;
        }

        fadeToBlackBackground.colorTint = new ColorAlpha(0, 0, 0, blinkingEffectTimeElapsed / blinkingEffectDuration);
        blinkingEffectTimeElapsed += Time.V_DeltaTime();
        
        if (blinkingEffectTimeElapsed > blinkingEffectDuration)
        {
            isAnimatingBlinkingEffect = false;
        }
    }

    /***********************************************************
       Interface to issue animations reqest..
    ***********************************************************/
    public void AnimateCrossHairFire()
    {
        isAnimatingCrossHair = true;
        crossHairTimeElasped = 0f;
        crossHairUi.scale = expandedCrossHairScale;
    }

    public void AnimateHealthLoss(float previousHealth, float currentHealth, float maxHealth)
    {
        float initialPercentage = previousHealth / maxHealth;
        float finalPercentage = currentHealth / maxHealth;

        // We are dealing with texture coordinates..
        initialPlayerHealthPercentage = new Vector2(1, initialPercentage);
        finalPlayerHealthPercentage = new Vector2(1, finalPercentage);

        initialHeathLossBufferPercentage = new Vector2(0, initialPercentage);
        finalHeathLossBufferPercentage = new Vector2(0, finalPercentage);

        if (healthDamageBuffer.textureCoordinatesStart.y > healthDamageBuffer.textureCoordinatesEnd.y)
        {   
            healthDamageBuffer.textureCoordinatesEnd = new Vector2(1, healthDamageBuffer.textureCoordinatesStart.y);
        }

        // We keep track of the first health loss..
        if (!isAnimatingHealthLoss)
        {
            lastHeathLossBufferPercentage = healthDamageBuffer.textureCoordinatesEnd;
        }

        isAnimatingHealthLoss = true;
        healthLossTimeElapsed = 0f;

        isWaitingForHealthRecovery = false;

        // Stop animating health gain..
        isAnimatingHealthGain = false;
        healthFill.colorTint = originalHealthGainColor;

        // Stop animating health buffer recovery..
        isAnimatingHealthBuffer = false;
    }

    public void AnimateHealthGain(float previousHealth, float currentHealth, float maxHealth)
    {
        float initialPercentage = previousHealth / maxHealth;
        float finalPercentage = currentHealth / maxHealth;

        // We are dealing with texture coordinates..
        initialPlayerHealthPercentage = new Vector2(1, initialPercentage);
        finalPlayerHealthPercentage = new Vector2(1, finalPercentage);

        initialHeathLossBufferPercentage = new Vector2(0, initialPercentage);
        finalHeathLossBufferPercentage = new Vector2(0, finalPercentage);

        // We keep track of the first health loss..
        if (!isAnimatingHealthGain)
        {
            lastHeathLossBufferPercentage = healthDamageBuffer.textureCoordinatesEnd;
        }

        isAnimatingHealthGain = true;
        healthGainTimeElapsed = 0f;

        // Stop animating health loss..
        isAnimatingHealthLoss = false;
        isWaitingForHealthRecovery = false;

        // Stop animating health buffer recovery..
        isAnimatingHealthBuffer = false;
    }

    public void SetDashUI(float currentDashStamina, float maxDashStamina)
    {
        float interval = (currentDashStamina / maxDashStamina) * 3f;

        for (int i = 0; i < dashFillBars.Count; ++i)
        {
            Image_ dashFillBar = dashFillBars[i];

            if (interval >= (i + 1))
            {
                dashFillBar.textureCoordinatesEnd = new Vector2(1, 1);
                dashFillBar.colorTint = readyDashFillColor;
            }
            else
            {
                float dashBarInterval = interval - i;
                dashFillBar.colorTint = originalDashFillColor;
                dashFillBar.textureCoordinatesEnd = new Vector2(dashBarInterval, 1);
            }
        }
    }

    public void SetQuestText(string text)
    {
        if (questText != null)
            questText.SetText(text);
    }

    public void SetCurrentAmmo(int currentAmmo)
    {
        int difference = currentAmmo - previousAmmoCount;
    
        if (difference > 0)
        {
            for(int i = previousAmmoCount; i < currentAmmo; ++i)
            {
                ammoFills[i].SetTexture(ammoFillTexture);
            } 
        }
        else if (difference < 0)
        {
            for (int i = currentAmmo; i < previousAmmoCount; ++i)
            {
                ammoFills[i].SetTexture(ammoUsedTexture);
            }
        }

        previousAmmoCount = currentAmmo;
    }

    public void SetUltimateBar(int previousSp, int currentSp, int maxSp)
    {
        if(previousSp == currentSp)
        {
            return;
        }

        float previousPercentage = (float) previousSp / (float) maxSp;
        float percentage = (float) currentSp / (float) maxSp;

        gunIsGlowing = false;
        gunUltimateGlow.gameObject.SetActive(false);
        gunUltimateFill.gameObject.SetActive(true);

        isAnimatingUltimateFill = true;
        initialUltimateFillPercentage = new Vector2(previousPercentage, 1);
        finalUltimateFillPercentage = new Vector2(percentage, 1);
        ultimateFillTimeElapsed = 0f;
        
    }

    /***********************************************************
       Tutorial Prompt
    ***********************************************************/
    public void ToggleTutorial()
    {
        isPaused = !isPaused;
        Systems.Pause = isPaused;
        tutorialUI?.SetActive(isPaused);

        if (isPaused)
        {
            tutorialUI?.getScript<TutorialPrompt>()?.BeginNextTutorial();
            CameraAPI.UnlockMouse();
            isTutorialPromptActive = true;
        }
        else
        {
            playerController.ResetWASDMovement();
            CameraAPI.LockMouse();
            isTutorialPromptActive = false;
        }
    }

    /***********************************************************
       Pause handler..
    ***********************************************************/
    public void PauseHandler()
    {
        if (playerIsDying)
        {
            return;
        }

        if (isTutorialPromptActive)
        {
            return;
        }

        Canvas_ settingsUI = GameObject.FindWithTag("Setting UI")?.getComponent<Canvas_>();

        if (settingsUI != null && settingsUI.isInteractable)
            return;

        isPaused = !isPaused;
        Systems.Pause = isPaused;

        pauseUI?.SetActive(isPaused);

        if (isPaused)
        {
            CameraAPI.UnlockMouse();
        }
        else
        {
            CameraAPI.LockMouse();
        }
    }
    
    /***********************************************************
       Dialogue 
    ***********************************************************/
    public void ActivateDialogue(string speaker, List<string> text, List<float> times, float finalDialogueTime)
    {
        dialogueScript.BeginDialogueSequence(speaker, text, times, finalDialogueTime);
    }

    /***********************************************************
       Death 
    ***********************************************************/
    public void TriggerDeathScreen()
    {
        if (playerIsDying)
        {
            return;
        }
        
        playerIsDying = true;
        CameraAPI.shakeCamera(0.5f, 1f);

        // Disable player UI..
        playerUI.SetActive(false);
        missionObjectiveUI.SetActive(false);

        // Show the screen crack overlay..
        deathScreenEffectUi.gameObject.SetActive(true);
        deathScreenEffectUi.SetTexture(glassCrackTextureOne);

        isAnimatingDeathSequenceRotation = true;
        deathSequencetimeElapsed = 0f;

        camera.toRotateSideWays = true;

        initialDeathRotation = camera.gameObject.transform.rotation;

        Vector3 eulerAngles = Rotation.ToEuler(initialDeathRotation);

        System.Random random = new System.Random();
        float angle = random.Next(0, 2) == 0 ? -90f * Mathf.Deg2Rad : 90f * Mathf.Deg2Rad;

        eulerAngles = new Vector3(eulerAngles.x, eulerAngles.y, angle);

        finalDeathRotation = Rotation.ToQuaternion(eulerAngles);

        RendererAPI.chromaticAberration = true;

        float chromaticAberrationDuration = totalDeathDuration;

        Interval(() =>
        {
            const float chromaticAberrationStrength = 0.01f;
            chromaticAberrationDuration -= chromaticAberrationInterval;

            RendererAPI.chromaticAberrationStrength = chromaticAberrationStrength * Mathf.Pow((chromaticAberrationDuration / totalDeathDuration), 2f);
            RendererAPI.randomiseChromaticAberrationOffset();
        }, chromaticAberrationInterval, totalDeathDuration);

        Invoke(() =>
        {
            fadeToBlackBackground.colorTint = new ColorAlpha(0, 0, 0, 0);
            RendererAPI.chromaticAberration = false;
            RendererAPI.blur = false;
            deathOverlay.SetActive(true);
            deathScreenEffectUi.gameObject.SetActive(false);

            Systems.Pause = true;
            CameraAPI.UnlockMouse();
        }, totalDeathDuration);
    }

    public void OnRestartButtonPressed()
    {
        Systems.Restart();

        // Restart();
        // RestartFromCheckpointButton?.Invoke();
    }
}