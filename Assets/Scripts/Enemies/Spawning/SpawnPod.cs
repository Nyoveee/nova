// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using ScriptingAPI;
using System;

public abstract class SpawnPod : Script
{
    protected Transform_ podTransform;
    protected Transform_ enemyTransform;
    protected Rigidbody_? enemyBody;

    protected bool animating = false;
    protected float timeElapsed = 0f;
    protected Vector3 EndPos;
    protected Vector3 StartPos;

    [SerializableField]
    protected float yOffset = 5f;
    [SerializableField]
    protected float duration = 2f;

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
            enemyTransform.position = Vector3.Lerp(StartPos, EndPos, Mathf.Pow(ratio, 0.15f));
        }
        else
        {
            animating = false;

            if (enemyBody != null)
                enemyBody.enable = true;

            OnAnimationFinished();
        }

        timeElapsed += Time.V_DeltaTime();
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

        EndPos = podTransform.position;
        StartPos = EndPos - new Vector3(0, yOffset, 0);

        enemyTrans.position = StartPos;

        Enemy enemyScript = enemyTrans.gameObject.getScript<Enemy>();
        enemyScript?.SetSpawningDuration(duration);
    }

    // overridden by subclasses
    protected abstract void OnAnimationFinished();

    // Lerp need duration
    // Script for animation
}