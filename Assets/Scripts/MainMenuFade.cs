// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class MainMenuFade : Script
{
    // Duration for logo fade in seconds
    public float logoFadeDuration = 1.5f;

    // Duration for buttons/text fade in seconds
    public float buttonsFadeDuration = 1.5f;

    public float alphaMultiplier = 0.8f;

    // References to the entities we need to fade
    public GameObject logoEntity;
    public GameObject playButtonEntity;
    public GameObject settingsButtonEntity;
    public GameObject quitButtonEntity;

    // Internal state tracking
    private float timeElapsed = 0f;
    private bool logoFadeComplete = false;
    private bool buttonsFadeComplete = false;
    private bool textShown = false;

    // Component references
    private Image_? logoImage;
    private Image_? playButtonImage;
    private Image_? settingsButtonImage;
    private Image_? quitButtonImage;

    // Text child entities
    private GameObject[]? playButtonChildren;
    private GameObject[]? settingsButtonChildren;
    private GameObject[]? quitButtonChildren;

    // This function is first invoked when game starts.
    protected override void init()
    {
        // Get the Image components from the entities
        logoImage = logoEntity.getComponent<Image_>();
        playButtonImage = playButtonEntity.getComponent<Image_>();
        settingsButtonImage = settingsButtonEntity.getComponent<Image_>();
        quitButtonImage = quitButtonEntity.getComponent<Image_>();

        // Get children (text entities) of each button
        playButtonChildren = playButtonEntity.GetChildren();
        settingsButtonChildren = settingsButtonEntity.GetChildren();
        quitButtonChildren = quitButtonEntity.GetChildren();

        // Hide all text children initially
        if (playButtonChildren != null)
        {
            for (int i = 0; i < playButtonChildren.Length; i++)
            {
                playButtonChildren[i].SetActive(false);
            }
        }

        if (settingsButtonChildren != null)
        {
            for (int i = 0; i < settingsButtonChildren.Length; i++)
            {
                settingsButtonChildren[i].SetActive(false);
            }
        }

        if (quitButtonChildren != null)
        {
            for (int i = 0; i < quitButtonChildren.Length; i++)
            {
                quitButtonChildren[i].SetActive(false);
            }
        }

        // Set all images to fully transparent at start with white color
        if (logoImage != null)
        {
            logoImage.colorTint = new ColorAlpha(1f, 1f, 1f, 0f);
        }

        if (playButtonImage != null)
        {
            ColorAlpha tint = playButtonImage.colorTint;
            playButtonImage.colorTint = new ColorAlpha(tint.r, tint.g, tint.b, 0f);
        }

        if (settingsButtonImage != null)
        {
            ColorAlpha tint = settingsButtonImage.colorTint;
            settingsButtonImage.colorTint = new ColorAlpha(tint.r, tint.g, tint.b, 0f);
        }

        if (quitButtonImage != null)
        {
            ColorAlpha tint = quitButtonImage.colorTint;
            quitButtonImage.colorTint = new ColorAlpha(tint.r, tint.g, tint.b, 0f);
        }
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        //timeElapsed += ;
        // Debug.Log(Time.V_DeltaTime());

        // Phase 1: Fade in the logo (0 to 1.5 seconds)
        if (!logoFadeComplete)
        {
            if (timeElapsed <= logoFadeDuration)
            {
                float alpha = timeElapsed / logoFadeDuration * alphaMultiplier;

                if (logoImage != null)
                {
                    logoImage.colorTint = new ColorAlpha(1f, 1f, 1f, alpha);
                }
            }
            else
            {
                // Logo fade complete, ensure it's fully opaque
                if (logoImage != null)
                {
                    logoImage.colorTint = new ColorAlpha(1f, 1f, 1f, alphaMultiplier);
                }
                logoFadeComplete = true;
            }
        }
        // Phase 2: Fade in the buttons (1.5 to 3.0 seconds)
        else if (!buttonsFadeComplete)
        {
            float buttonsElapsed = timeElapsed - logoFadeDuration;

            // Show text at the start of button fade
            if (!textShown)
            {
                if (playButtonChildren != null)
                {
                    for (int i = 0; i < playButtonChildren.Length; i++)
                    {
                        playButtonChildren[i].SetActive(true);
                    }
                }

                if (settingsButtonChildren != null)
                {
                    for (int i = 0; i < settingsButtonChildren.Length; i++)
                    {
                        settingsButtonChildren[i].SetActive(true);
                    }
                }

                if (quitButtonChildren != null)
                {
                    for (int i = 0; i < quitButtonChildren.Length; i++)
                    {
                        quitButtonChildren[i].SetActive(true);
                    }
                }

                textShown = true;
            }

            if (buttonsElapsed <= buttonsFadeDuration)
            {
                float alpha = buttonsElapsed / buttonsFadeDuration;

                if (playButtonImage != null)
                {
                    ColorAlpha tint = playButtonImage.colorTint;
                    playButtonImage.colorTint = new ColorAlpha(tint.r, tint.g, tint.b, alpha);
                }

                if (settingsButtonImage != null)
                {
                    ColorAlpha tint = settingsButtonImage.colorTint;
                    settingsButtonImage.colorTint = new ColorAlpha(tint.r, tint.g, tint.b, alpha);
                }

                if (quitButtonImage != null)
                {
                    ColorAlpha tint = quitButtonImage.colorTint;
                    quitButtonImage.colorTint = new ColorAlpha(tint.r, tint.g, tint.b, alpha);
                }
            }
            else
            {
                // Buttons fade complete, ensure they're fully opaque
                if (playButtonImage != null)
                {
                    ColorAlpha tint = playButtonImage.colorTint;
                    playButtonImage.colorTint = new ColorAlpha(tint.r, tint.g, tint.b, 1f);
                }

                if (settingsButtonImage != null)
                {
                    ColorAlpha tint = settingsButtonImage.colorTint;
                    settingsButtonImage.colorTint = new ColorAlpha(tint.r, tint.g, tint.b, 1f);
                }

                if (quitButtonImage != null)
                {
                    ColorAlpha tint = quitButtonImage.colorTint;
                    quitButtonImage.colorTint = new ColorAlpha(tint.r, tint.g, tint.b, 1f);
                }

                buttonsFadeComplete = true;
            }
        }
    }
}
