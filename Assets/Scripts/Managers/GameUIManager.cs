// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class GameUIManager : Script
{
    public enum ProgressBarType
    {
        HealthBar,
        DashBar
    }
    private float currentFadeTime;
    private Dictionary<ProgressBarType, Transform_> progressBars = new Dictionary<ProgressBarType, Transform_>();
    private Dictionary<ProgressBarType, float> progressMaxSize = new Dictionary<ProgressBarType, float>();

    private Dictionary<Type, Transform_> gunToUiContainer = new Dictionary<Type, Transform_>();

    /***********************************************************
        Inspector Variables
    ***********************************************************/
    [SerializableField]
    private Transform_? dashBar = null;
    [SerializableField]
    private Transform_? healthBar = null;
    [SerializableField]
    private Image_? image = null;
    [SerializableField]
    private Text_? healthText = null;
    [SerializableField]
    private float damageFadeTime = 2.0f;

    [SerializableField]
    private Transform_ sniperContainerUi;
    [SerializableField]
    private Transform_ shotgunContainerUi;

#if false
    [SerializableField]
    private ColorAlpha activeGunOverlayColor;
    [SerializableField]
    private ColorAlpha inactiveGunOverlayColor;
#endif

    protected override void init()
    {
        if(dashBar != null && healthBar!= null)
        {
            progressBars[ProgressBarType.DashBar] = dashBar;
            progressBars[ProgressBarType.HealthBar] = healthBar;
            progressMaxSize[ProgressBarType.DashBar] = dashBar.scale.x;
            progressMaxSize[ProgressBarType.HealthBar] = healthBar.scale.x;
        }

        gunToUiContainer.Add(typeof(Sniper), sniperContainerUi);
        gunToUiContainer.Add(typeof(Shotgun), shotgunContainerUi);
    }

    protected override void update()
    {
        if(image != null && image.colorTint.a > 0)
        {
            currentFadeTime = Mathf.Max(currentFadeTime - Time.V_FixedDeltaTime(), 0);
            ColorAlpha colora = image.colorTint;
            if(damageFadeTime <= 0)
            {
                Debug.LogError("Damage Fade Time must be more than zero");
                return;
            }
            colora.a = Mathf.Interpolate(0, 1, currentFadeTime / damageFadeTime, 1);
            image.colorTint = colora;
        }
    }
    /***********************************************************
       UI Setters
   ***********************************************************/
    public void SetProgress(ProgressBarType progressBarType, float current, float max)
    {
        if (max == 0){
            Debug.LogError("Progress bar max cannot be zero");
            return;
        }
        Vector3 scale = progressBars[progressBarType].scale;
        scale.x = Mathf.Interpolate(0, progressMaxSize[progressBarType], current / max,1);
        progressBars[progressBarType].scale = scale;
    }
    public void ActivateDamageUI()
    {
        if (image != null)
        {
            ColorAlpha colora = image.colorTint;
            colora.a = 1;
            image.colorTint = colora;
            currentFadeTime = damageFadeTime;
        }
    }
    public void SetHealthText(int health)
    {
        if(healthText!= null){
            healthText.SetText(health.ToString());
        }
    }

    public void SwapWeaponUI(Gun from, Gun to)
    {
        Transform_ fromUiContainer = gunToUiContainer[from.GetType()];
        Transform_ toUiContainer = gunToUiContainer[to.GetType()];

        Debug.Log(Vector3.One());

        fromUiContainer.localScale = Vector3.One();

        foreach(GameObject child in fromUiContainer.gameObject.GetChildren())
        {
            Image_ overlayImage = child.getComponent<Image_>();

            if (overlayImage != null)
                overlayImage.colorTint = new ColorAlpha(125f / 255f, 125f / 255f, 125f / 255f, 0.5f);
        }

        toUiContainer.localScale = Vector3.One() * 1.3f;

        foreach (GameObject child in toUiContainer.gameObject.GetChildren())
        {
            Image_ overlayImage = child.getComponent<Image_>();

            if (overlayImage != null)
                overlayImage.colorTint = new ColorAlpha(10f / 255f, 10f / 255f, 10f / 255f, 0.6f);
        }
    }
}