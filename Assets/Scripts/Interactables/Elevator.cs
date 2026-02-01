using System;

class Elevator : Script
{
    /**********************************************************************
        Inspector Variables
    **********************************************************************/
    [SerializableField]
    private Transform_ elevatorTransform;
    [SerializableField]
    private float moveHeight;
    [SerializableField]
    private float elevatorMoveDuration = 2f;
    [SerializableField]
    private Door tutorialDoor;
    [SerializableField]
    private Door hubDoor;
    [SerializableField]
    private GameObject playerBody;

    /**********************************************************************
        Local Variables
    **********************************************************************/
    Vector3 topPosition;
    Vector3 bottomPosition;
    private enum ElevatorState
    {
        Idle,
        MovingDown,
    }
    private delegate void CurrentState();
    private ElevatorState elevatorState = ElevatorState.Idle;
    private Dictionary<ElevatorState, CurrentState> updateState = new Dictionary<ElevatorState, CurrentState>();
    private float currentElevatorMoveTime;
    private bool b_reachedHub = false;

    protected override void init()
    {
        updateState.Add(ElevatorState.Idle, Update_Idle);
        updateState.Add(ElevatorState.MovingDown, Update_MovingDown);
        topPosition = elevatorTransform.position;
        bottomPosition = topPosition - new Vector3(0, moveHeight, 0);
        tutorialDoor.OpenDoor();
    }

    protected override void update()
    {
        updateState[elevatorState]();
    }
    private void Update_Idle() {
        if (tutorialDoor.IsFullyClosed() && !b_reachedHub)
            elevatorState = ElevatorState.MovingDown;
    }
    private void Update_MovingDown() {
        float t = currentElevatorMoveTime / elevatorMoveDuration;
        currentElevatorMoveTime += Time.V_DeltaTime();

        Vector3 prevPos = elevatorTransform.position;
        elevatorTransform.position = Vector3.Lerp(topPosition, bottomPosition, t);

        Vector3 distanceMoved = elevatorTransform.position - prevPos;
        playerBody.getComponent<Transform_>().position += distanceMoved;
        //playerBody.getComponent<Rigidbody_>().AddVelocity(distanceMoved);

        if (t >= 1f)
        {
            elevatorState = ElevatorState.Idle;
            hubDoor.OpenDoor();
            b_reachedHub = true;
        }
    }
    /**********************************************************************
        Collision Trigger Events
    **********************************************************************/
    public void CloseTutorialDoor() {
        tutorialDoor.CloseDoor();
    }
}