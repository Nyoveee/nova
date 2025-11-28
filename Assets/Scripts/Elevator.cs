using System;

class Elevator : Script
{
    private enum DoorState
    {
        Closed,
        Opening,
        Open,
        Closing
    }

    private DoorState doorState = DoorState.Closed;
    private DoorState HubDoorState = DoorState.Closed;

    // Left and right door panels
    [SerializableField]
    private Transform_ TutorialLeftDoor;
    [SerializableField]
    private Transform_ TutorialRightDoor;

    // Left and right door panels
    [SerializableField]
    private Transform_ HubLeftDoor;
    [SerializableField]
    private Transform_ HubRightDoor;
    [SerializableField]
    private Vector3 openOffset = new Vector3(10, 0, 0);

    // Positions
    private Vector3 tLeftStart;
    private Vector3 tLeftEnd;

    private Vector3 tRightStart;
    private Vector3 tRightEnd;

    // Positions
    private Vector3 hLeftStart;
    private Vector3 hLeftEnd;

    private Vector3 hRightStart;
    private Vector3 hRightEnd;

    private float openSpeed = 2f;
    private float t = 0f;

    private Transform_ leftDoor;
    private Transform_ rightDoor;
    private Vector3 leftStart;
    private Vector3 leftEnd;
    private Vector3 rightStart;
    private Vector3 rightEnd;

    // Elevator
    private enum ElevatorState
    {
        Idle,
        MovingUp,
        MovingDown,
    }

    private ElevatorState elevatorState = ElevatorState.Idle;

    [SerializableField]
    private Transform_ elevator;

    // Editable in editor
    [SerializableField]
    private Vector3 topPosition;     // Default floor
    [SerializableField]
    private Vector3 bottomPosition;

    [SerializableField]
    private float moveHeight = 10f;

    private float moveSpeed = 1f;
    private float moveT = 0f;

    private bool atTop = true;

    protected override void init()
    {
        // Setup both door panels
        tLeftStart = TutorialLeftDoor.position;
        tLeftEnd = tLeftStart - openOffset;

        tRightStart = TutorialRightDoor.position;
        tRightEnd = tRightStart + openOffset;   // opposite direction

        hLeftStart = HubLeftDoor.position - new Vector3(0,25,0);
        hLeftEnd = hLeftStart + openOffset;
        
        hRightStart = HubRightDoor.position - new Vector3(0, 25, 0);
        hRightEnd = hRightStart - openOffset;   // opposite direction

        Console.WriteLine("Double Door initialized. State: " + doorState);
        Console.WriteLine("Double Door initialized. State: " + HubDoorState);

        // Setup Elevator
        //elevator.position = topPosition;
        topPosition = elevator.position;
        bottomPosition = topPosition - new Vector3(0, moveHeight, 0);

        atTop = true;
        elevatorState = ElevatorState.Idle;
        Console.WriteLine("Elevator initialized at TOP floor. State: " + elevatorState);
    }

    protected override void update()
    {
        // Choose which doors are active
        if (atTop)
        {
            // Top floor, tutorial doors
            leftDoor = TutorialLeftDoor;
            rightDoor = TutorialRightDoor;

            leftStart = tLeftStart;
            leftEnd = tLeftEnd;
            rightStart = tRightStart;
            rightEnd = tRightEnd;
        }
        else
        {
            // Bottom floor hub doors
            leftDoor = HubLeftDoor;
            rightDoor = HubRightDoor;

            leftStart = hLeftStart;
            leftEnd = hLeftEnd;
            rightStart = hRightStart;
            rightEnd = hRightEnd;
        }

        AnimateDoor();
        AnimateElevator();
    }

    private void AnimateDoor()
    {
        if (doorState == DoorState.Opening)
        {
            t += Time.V_FixedDeltaTime() * openSpeed;
            leftDoor.position = Vector3.Lerp(leftStart, leftEnd, t);
            rightDoor.position = Vector3.Lerp(rightStart, rightEnd, t);

            if (t >= 1f)
                doorState = DoorState.Open;
        }
        else if (doorState == DoorState.Closing)
        {
            t += Time.V_FixedDeltaTime() * openSpeed;
            leftDoor.position = Vector3.Lerp(leftEnd, leftStart, t);
            rightDoor.position = Vector3.Lerp(rightEnd, rightStart, t);

            if (t >= 1f)
                doorState = DoorState.Closed;
        }

        if (doorState == DoorState.Closed)
        {
            t = 0f;
        }
        else if (doorState == DoorState.Open)
        {
            t = 0f;
        }
    }

    private void AnimateElevator()
    {
        if (elevatorState == ElevatorState.MovingUp)
        {
            moveT += Time.V_FixedDeltaTime() * moveSpeed;
            elevator.position = Vector3.Lerp(bottomPosition, topPosition, moveT);

            if (moveT >= 1f)
            {
                atTop = true;
                elevatorState = ElevatorState.Idle;
            }
        }
        else if (elevatorState == ElevatorState.MovingDown)
        {
            moveT += Time.V_FixedDeltaTime() * moveSpeed;
            elevator.position = Vector3.Lerp(topPosition, bottomPosition, moveT);

            if (moveT >= 1f)
            {
                atTop = false;
                elevatorState = ElevatorState.Idle;
            }
        }
    }

    public void MoveElevatorUp()
    {
        if (atTop || elevatorState != ElevatorState.Idle)
            return;

        moveT = 0f;
        elevatorState = ElevatorState.MovingUp;
        Console.WriteLine("State: " + elevatorState);
    }

    public void MoveElevatorDown()
    {
        if (!atTop || elevatorState != ElevatorState.Idle)
            return;

        moveT = 0f;
        elevatorState = ElevatorState.MovingDown;
        Console.WriteLine("State: " + elevatorState);
    }

    // Public functions for other scripts to call
    public void OpenDoors()
    {
        if (doorState == DoorState.Open || doorState == DoorState.Opening)
            return;

        doorState = DoorState.Opening;
        t = 0f;
    }

    public void CloseDoors()
    {
        if (doorState == DoorState.Closed || doorState == DoorState.Closing)
            return;

        doorState = DoorState.Closing;
        t = 0f;
    }

    public bool IsDoorOpen() => doorState == DoorState.Open;
    public bool IsDoorClosed() => doorState == DoorState.Closed;
    public bool IsAtTop() => atTop;
    public bool IsAtBottom() => !atTop;
}