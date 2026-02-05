// Make sure the class name matches the filepath, without space!!.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.
class DialogueScript : Script
{
    private Vector3 startPosition;
    private Vector3 endPosition;
    private float currentTransitionTime;
    private float finalDialogueTime;
    private List<string> dialogue;
    int currentIndex;
    /***********************************************************
        Inspector Variables
    ***********************************************************/
    [SerializableField]
    private Transform_ dialogueTextBackgroundTransform;
    [SerializableField]
    private float dialogueBoxTransitionTime;
    [SerializableField]
    private Text_ dialogueText;
    [SerializableField]
    private Text_ speakerText;

    private enum DialogueState
    {
        StartTransition,
        InDialogue,
        EndTransition
    }
    private DialogueState dialogueState;
    protected override void update()
    {
        currentTransitionTime -= Time.V_DeltaTime();
        currentTransitionTime = Mathf.Max(currentTransitionTime, 0);
        if (dialogueState == DialogueState.StartTransition)
        {
            gameObject.transform.position = Vector3.Lerp(endPosition, startPosition, currentTransitionTime / dialogueBoxTransitionTime);
            if (currentTransitionTime <= 0)
                dialogueState = DialogueState.InDialogue;
        }
        if(dialogueState == DialogueState.InDialogue)
        {
            if (currentIndex != dialogue.Count)
                return;
            finalDialogueTime -= Time.V_DeltaTime();
            if(finalDialogueTime < 0)
            {
                currentTransitionTime = dialogueBoxTransitionTime;
                dialogueState = DialogueState.EndTransition;
                return;
            }
           
        }
        if(dialogueState == DialogueState.EndTransition)
        {
            gameObject.transform.position = Vector3.Lerp(startPosition, endPosition, currentTransitionTime / dialogueBoxTransitionTime);
            if(currentTransitionTime <= 0)
                gameObject.SetActive(false);
        }
    }
    public void BeginDialogueSequence(string speaker, List<string> text, List<float> times, float finalDialogueTime)
    {
        // Set the position
        startPosition = new Vector3(Systems.ScreenResolution.x + dialogueTextBackgroundTransform.scale.x, 0, 0);
        endPosition = gameObject.transform.position;
        gameObject.transform.position = startPosition;
        // Set the Dialogues to run
        dialogueState = DialogueState.StartTransition;
        dialogue = text;
        currentIndex = 0;
        speakerText.SetText(speaker);
        currentTransitionTime = dialogueBoxTransitionTime;
        // There's a chance if this function is called during another dialogue it might mess up due to invokes still existing
        // if that's the case can replace with a normal update
        for (int i = 0;i < dialogue.Count; ++i){
            Invoke(() =>
            {
                if (currentIndex == dialogue.Count - 1)
                    this.finalDialogueTime = finalDialogueTime;
                dialogueText.SetText(dialogue[currentIndex++]);
            }, times[i]);
        }

     
    }

}