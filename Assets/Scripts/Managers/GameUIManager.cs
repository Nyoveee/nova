// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using static GameUIManager;

class   GameUIManager : Script
{
    public enum ProgressBarType
    {
        HealthBar,
        DashBar,
        UltimateBar
    }
    private float currentFadeTime;
    private Dictionary<ProgressBarType, Image_> progressBars = new ();

    private bool isPaused = false;  
    /***********************************************************
        Inspector Variables
    ***********************************************************/
    [SerializableField]
    private Image_? dashBar = null;
    [SerializableField]
    private Image_? healthBar = null;
    [SerializableField]
    private Image_? ultimateBar = null;

    [SerializableField]
    private GameObject? pauseUI = null;

    [SerializableField]
    private Image_? damageBackground = null;
    [SerializableField]
    private Text_? questText = null;
    [SerializableField]
    private float damageFadeTime = 2.0f;

    [SerializableField]
    private Text_? maxAmmoText = null;

    [SerializableField]
    private Text_? currentAmmoText = null;

    protected override void init()
    {
        progressBars[ProgressBarType.DashBar] = dashBar;
        progressBars[ProgressBarType.HealthBar] = healthBar;
        progressBars[ProgressBarType.UltimateBar] = ultimateBar;

        MapKey(Key.P, PauseHandler, true);
    }

    protected override void update()
    {
        if(damageBackground != null && damageBackground.colorTint.a > 0)
        {
            currentFadeTime = Mathf.Max(currentFadeTime - Time.V_DeltaTime(), 0);
            ColorAlpha colora = damageBackground.colorTint;
            if(damageFadeTime <= 0)
            {
                Debug.LogError("Damage Fade Time must be more than zero");
                return;
            }
            colora.a = Mathf.Interpolate(0, 1, currentFadeTime / damageFadeTime, 1);
            damageBackground.colorTint = colora;
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

        if(progressBars[progressBarType] == null)
        {
            return;
        }

        Vector2 textureCoordinates = progressBars[progressBarType].textureCoordinatesRange;
        textureCoordinates.x = Mathf.Interpolate(0, 1f, current / max, 1);
        progressBars[progressBarType].textureCoordinatesRange = textureCoordinates;
    }

    public void ActivateDamageUI()
    {
        if (damageBackground != null)
        {
            ColorAlpha colora = damageBackground.colorTint;
            colora.a = 1;
            damageBackground.colorTint = colora;
            currentFadeTime = damageFadeTime;
        }
    }

    public void SetQuestText(string text)
    {
        if (questText != null)
            questText.SetText(text);
    }

    public void SetAmmoText(int currentAmmo, int maxAmmo)
    {
        SetCurrentAmmoText(currentAmmo);

        if (maxAmmoText != null)
            maxAmmoText.SetText("/ " + currentAmmo);
    }

    public void SetCurrentAmmoText(int currentAmmo)
    {
        if (currentAmmoText != null)
            currentAmmoText.SetText(currentAmmo.ToString());
    }

    public void SetUltimateBarUI(int currentSp, int maxSp)
    {
        if (maxSp == 0)
        {
            Debug.LogError("Progress bar max cannot be zero");
            return;
        }

        if (ultimateBar == null)
        {
            return;
        }

        Vector2 textureCoordinates = ultimateBar.textureCoordinatesRange;
        textureCoordinates.x = Mathf.Interpolate(0, 1f, currentSp / maxSp, 1);
        ultimateBar.textureCoordinatesRange = textureCoordinates;
    }

    /***********************************************************
       Pause handler..
    ***********************************************************/
    public void PauseHandler()
    {
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
}