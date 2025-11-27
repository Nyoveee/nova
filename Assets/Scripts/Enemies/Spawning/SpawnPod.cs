// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;

public abstract class SpawnPod : Script
{
    protected Transform_ podTransform;
    protected Transform_ enemyTransform;
    protected Rigidbody_? enemyBody;

    protected bool animating = false;
    protected float timeElapsed = 0f;
    protected Vector3 EndPos;

    [SerializableField]
    protected float yOffset = 5f;
    [SerializableField]
    protected float duration = 2f;

    [SerializableField]
    protected Wave wave;

    protected override void init()
    {
        podTransform = getComponent<Transform_>();
    }

    protected override void update()
    {
        if (!animating || enemyTransform == null)
            return;

        float ratio = timeElapsed / duration;

        if (ratio < 1)
        {
            enemyTransform.position = Vector3.Lerp(podTransform.position, EndPos, ratio);
        }
        else
        {
            animating = false;

            if (enemyBody != null)
                enemyBody.enable = true;

            OnAnimationFinished();
        }

        timeElapsed += Time.V_FixedDeltaTime();

    }

    protected override void exit()
    {
        if (enemyBody != null)
            enemyBody.enable = true;
    }

    protected void StartAnimation(Transform_ enemyTrans)
    {
        enemyTransform = enemyTrans;
        enemyBody = enemyTrans.gameObject.getComponent<Rigidbody_>();
        if (enemyBody != null)
            enemyBody.enable = false;

        timeElapsed = 0f;
        animating = true;

        // testing gravity/rigid body
        EndPos = podTransform.position;
        EndPos.y += yOffset;
    }

    // overridden by subclasses
    protected abstract void OnAnimationFinished();

    // Lerp need duration
    // Script for animation
}