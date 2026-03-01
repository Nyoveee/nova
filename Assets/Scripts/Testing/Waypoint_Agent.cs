// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class Waypoint_Agent : Script
{


    public enum AgentState
    { 
        Idle,
        PathFinding
    
    }

    public List<Vector3> waypoints;
    public int index = 0;

    public float stopDistance = 1.0f;

    public float moveSpeed = 2.0f;

    private AgentState state = AgentState.Idle;

    // This function is invoked once when gameobject is active.
    protected override void init()
    {
    }

    // This function is invoked every update.
    protected override void update()
    {
        switch(state)
        {
            case AgentState.Idle:
                { 
                
                }
                break;
            case AgentState.PathFinding:
                {
                    MoveTo();
                
                }
                break;
        }
    
    
    }

    // This function is invoked every update.
    protected override void fixedUpdate()
    {}


    public void SetNewPosition(Vector3 pos)
    { 

        index = 0;

        waypoints = NavigationAPI.CalculatePath("Humanoid", gameObject.transform.position, pos);

        if(waypoints != null && waypoints.Count > 0)
        {
            Debug.Log("Path Found! Waypoints numbers: " + waypoints.Count);
            state = AgentState.PathFinding;
        }

       // Debug.Log("Position: " + pos + " Waypoints numbers: " + waypoints.Count);


    }


    public void MoveTo()
    { 
        
        if (Vector3.Distance(gameObject.transform.position, waypoints[index]) < stopDistance)
        {
            index += 1;

            Debug.Log("Waypoint Reach! ");
            if (index >= waypoints.Count)
            {
                state = AgentState.Idle;

                return;
            }
        }

        Debug.Log("Moving: " + gameObject.transform.position);

        Vector3 direction =  waypoints[index] - gameObject.transform.position;
        direction.Normalize();

        gameObject.transform.position += direction * moveSpeed * Time.V_DeltaTime();
    
    }


}