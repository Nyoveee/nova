using System;

class Door : Script
{
    private enum DoorState
    {
        Closed,
        Opening,
        Open,
        Closing,
        Locked,
        Unlocked,
    }

    private DoorState state = DoorState.Closed;

    public Transform_ player;

    // Left and right door panels
    public Transform_ leftDoor;
    public Transform_ rightDoor;

    // Editable 
    public Vector3 openOffset = new Vector3(0, 0, 2);
    public float detectionRange = 10f;

    // Positions
    private Vector3 leftStart;
    private Vector3 leftEnd;

    private Vector3 rightStart;
    private Vector3 rightEnd;

    private float openSpeed = 2f;
    private float t = 0f;

    protected override void init()
    {
        // Setup both door panels
        leftStart = leftDoor.position;
        leftEnd = leftStart + openOffset;

        rightStart = rightDoor.position;
        rightEnd = rightStart - openOffset;   // opposite direction

        Console.WriteLine("Double Door initialized. State: " + state);
    }

    protected override void update()
    {
        float distance = Vector3.Distance(player.position, leftStart);

        if (distance < detectionRange && state == DoorState.Closed)
        {
            state = DoorState.Opening;
            t = 0f;
        }
        else if (distance > detectionRange && state == DoorState.Open)
        {
            state = DoorState.Closing;
            t = 0f;
        }

        AnimateDoor();
    }


    private void AnimateDoor()
    {
        if (state == DoorState.Opening)
        {
            t += Time.V_FixedDeltaTime() * openSpeed;

            leftDoor.position = Vector3.Lerp(leftStart, leftEnd, t);
            rightDoor.position = Vector3.Lerp(rightStart, rightEnd, t);

            if (t >= 1f)
                state = DoorState.Open;
        }
        else if (state == DoorState.Closing)
        {
            t += Time.V_FixedDeltaTime() * openSpeed;

            leftDoor.position = Vector3.Lerp(leftEnd, leftStart, t);
            rightDoor.position = Vector3.Lerp(rightEnd, rightStart, t);

            if (t >= 1f)
                state = DoorState.Closed;
        }
    }
}