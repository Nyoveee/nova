// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
using WinRT;

class PathfindingCube : Script
{
    public int cubeIndex = -1;
    public bool isCubeControllable = false;

    public bool  jumpState = false;
    public float jumpDuration = 1.0f;
    public float jumpHeight = 0.0f;
    private float jumpHeightMax = 0;
    public float timeElapsed = 0.0f;

    private Vector3 startPos;
    private Vector3 endPos;

    // This function is first invoked when game starts.
    protected override void init()
    {
      MapKey(Key.MouseLeft, OnMouseClick);
        // MapKey(Key.S, OnMouseClick);
        getComponent<NavMeshAgent_>().setAutomateNavMeshOfflinksState(false);
    }

    // This function is invoked every fixed update.
    protected override void update()
    {
        //if (isCubeControllable == false)
        //{
        //    return;
        //}

        //Jump Code
        if (jumpState == true)
        {
            float t = timeElapsed / jumpDuration;

            Vector3 horizontalPos = Vector3.Lerp(startPos, endPos, t);

            gameObject.transform.position = horizontalPos;

            float yOffset = jumpHeightMax * 4f * (t - t * t);
            gameObject.transform.position = horizontalPos + Vector3.Up() * yOffset;

            timeElapsed += Time.V_DeltaTime();
            //timeElapsed += 0.00000000001f;

            if (timeElapsed >= jumpDuration)
            {
                jumpState = false;
                timeElapsed = 0.0f;
                getComponent<NavMeshAgent_>().CompleteOffMeshLink();
            }
        }

        //Ray ray = CameraAPI.getRayFromMouse();
        //RayCastResult? result = PhysicsAPI.Raycast(ray, 1000f);

        //if (result != null)
        //{
        //    Debug.Log("ray hit at " + result.Value.point + ", hitting entity " + result.Value.entity);
        //    NavigationAPI.setDestination(gameObject, result.Value.point);
        //}
        if ( getComponent<NavMeshAgent_>().isOnOffMeshLinks() == true && jumpState == false)
        {
            NavMeshOfflinkData data = getComponent<NavMeshAgent_>().getOffLinkData();

            if (data.valid == true)
            {

                startPos = data.startNode;
                jumpState = true;
                endPos = data.endNode;
                timeElapsed = 0.0f;
                float verticalDiff = endPos.y - startPos.y;
                jumpHeightMax = Mathf.Max(0.5f * Math.Abs(verticalDiff), 0.25f) + jumpHeight; // 0.25f adds small curve
            }
        }
        

    }


    private void OnMouseClick() 
    {

        if (isCubeControllable == false)
        {
            return;
        }


        Ray ray = CameraAPI.getRayFromMouse();
        RayCastResult? result = PhysicsAPI.Raycast(ray, 1000f);

        if (result != null)
        {
            Debug.Log("ray hit at " + result.Value.point + ", hitting entity " + result.Value.entity);
            NavigationAPI.setDestination(gameObject, result.Value.point);


           //gameObject.getComponent<NavMeshAgent_>().Warp(result.Value.point);
        }

    }


}