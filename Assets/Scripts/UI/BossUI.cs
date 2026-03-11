// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class BossUI : Script
{
    // ========================================================================
    // References to game objects..

    [SerializableField]
    private Canvas_ bossUI;

    [SerializableField]
    private Image_ bossHealthFill;

    // ========================================================================
    // Heath loss animation configuration..

    [SerializableField]
    private float healthLossLerpDuration = 0.7f;

    [SerializableField]
    private float healthLossLerpPower = 0.4f;

    // ========================================================================
    // ========================================================================
    // Runtime variables..

    private float healthLossTimeElapsed = 0f;
    private bool isAnimatingHealthLoss = false;

    private Vector2 initialHealthUV;
    private Vector2 finalHealthUV;

    // ========================================================================
    protected override void update()
    {
        AnimatingHealthLoss();
    }

    private void AnimatingHealthLoss()
    {
        // Animating health loss..
        if (!isAnimatingHealthLoss)
        {
            return;
        }

        bossHealthFill.textureCoordinatesEnd = Vector2.Lerp(initialHealthUV, finalHealthUV, Mathf.Pow(healthLossTimeElapsed / healthLossLerpDuration, healthLossLerpPower));

        healthLossTimeElapsed += Time.V_DeltaTime();

        if (healthLossTimeElapsed > healthLossLerpDuration)
        {
            isAnimatingHealthLoss = false;
        }
    }

    // ========================================================================
    // Public API..

    public void SetBossHealth(float previousHealth, float currentHealth, float maxHealth)
    {
        if (currentHealth < 0)
        {
            bossUI.gameObject.SetActive(false);
            return;
        }

        float currentPercentage = currentHealth / maxHealth;
        float previousPercentage = previousHealth / maxHealth;

        initialHealthUV = new Vector2(previousPercentage, 1f);
        finalHealthUV = new Vector2(currentPercentage, 1f);

        isAnimatingHealthLoss = true;
        healthLossTimeElapsed = 0f;
    }
}