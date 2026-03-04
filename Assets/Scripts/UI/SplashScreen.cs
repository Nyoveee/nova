// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using Windows.AI.MachineLearning;
class SplashScreen : Script
{
    enum SplashScreenState
    {
        FadeIn,
        Delay,
        FadeOut,
    }
    [SerializableField]
    private Scene nextScene;
    [SerializableField]
    private GameObject imageListGameObject;
    [SerializableField]
    private float fadeInTime;
    [SerializableField]
    private float delayTime;
    [SerializableField]
    private float fadeOutTime;

    private float currentFadeTime = 0;
    private int currentImageIndex = 0;
    private SplashScreenState currentFadeState = SplashScreenState.FadeIn;
    private List<Image_> images = new List<Image_>();
    protected override void init()
    {
        foreach(GameObject imageGo in imageListGameObject.GetChildren())
        {
            Image_ image = imageGo.getComponent<Image_>();
            images.Add(image);
            image.colorTint = new ColorAlpha(1f, 1f, 1f, 0);
        }
        MapKey(Key.Escape, SkipCurrentLogo);
    }
    protected override void update()
    {
        currentFadeTime += Time.V_DeltaTime();
        switch (currentFadeState)
        {
            case SplashScreenState.FadeIn:
            {
                if (currentFadeTime >= fadeInTime)
                {
                    currentFadeTime = 0;
                    currentFadeState = SplashScreenState.Delay;
                    break;
                }
                float alpha = Mathf.Interpolate(0, 1, currentFadeTime / fadeInTime, 1);
                images[currentImageIndex].colorTint = new ColorAlpha(1f, 1f, 1f, alpha);
                break;
            }
            case SplashScreenState.Delay:
            {
                if(currentFadeTime>= delayTime)
                {
                    currentFadeTime = 0;
                    currentFadeState = SplashScreenState.FadeOut;
                }
                break;
            }
            case SplashScreenState.FadeOut:
            {
                if (currentFadeTime >= fadeOutTime)
                {
                    ++currentImageIndex;
                    if (currentImageIndex == images.Count)
                        SceneAPI.ChangeScene(nextScene);
                    currentFadeTime = 0;
                    currentFadeState = SplashScreenState.FadeIn;
                    break;
                }
                float alpha = Mathf.Interpolate(1, 0, currentFadeTime / fadeOutTime, 1);
                images[currentImageIndex].colorTint = new ColorAlpha(1f, 1f, 1f, alpha);
                break;
            }
        }
    }
    private void SkipCurrentLogo()
    {
        if (currentFadeState == SplashScreenState.FadeIn)
        {
            float ratio = currentFadeTime / fadeInTime;
            currentFadeTime = ratio * fadeOutTime;
        }
        else if (currentFadeState == SplashScreenState.Delay)
            currentFadeTime = 0;
        currentFadeState = SplashScreenState.FadeOut;
    }
}