// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using Windows.System.Threading;

class SpinningCat : Script
{
    public float angle = 0;

    private Transform_? transform;

    public float spinningDuration = 2;
    public float spinningCooldown = 1;

    // ==============================
    private bool isSpinning;
    private float timeElapsed = 0;
    
    // This function is first invoked when game starts.
    protected override void init()
    {
        transform = getComponent<Transform_>();
        startSpinning();
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        if (isSpinning && transform != null) 
        { 
            if(transform!= null)
            {
                transform.rotate(transform.up, angle * Time.V_DeltaTime());
            }
           

            if(timeElapsed > spinningDuration)
            {
                timeElapsed = 0;
                stopSpinning();
                return;
            }
        }
        else
        {
            if (timeElapsed > spinningCooldown)
            {
                timeElapsed = 0;
                startSpinning();
                return;
            }

            // countdown before next spin.
        }

        timeElapsed += Time.V_DeltaTime();
    }

    private void startSpinning()
    {
        isSpinning = true;
        AudioAPI.PlaySound(gameObject, "oiia-oiia-sound");
    }

    private void stopSpinning()
    {
        isSpinning = false;
    }

}