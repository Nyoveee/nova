// Make sure the class name matches the asset name.
// If you want to change class name, change the asset name in the editor!
// Editor will automatically rename and recompile this file.


public class QuestManager : Script
{
    private List<Quest> quests = new List<Quest>();
    private Quest? currentQuest;
    private int questIndex;
    private PlayerController_V2? player;
    private GameUIManager gameUIManager;

    [SerializableField]
    private GameObject questContainer;

    [SerializableField]
    private GameObject playerHead;
    [SerializableField]
    private GameObject playerOrientation;

    protected override void init()
    {
        gameUIManager = GameObject.FindWithTag("Game UI Manager")?.getScript<GameUIManager>();
        GameObject[] children = gameObject.GetChildren();

        foreach (var child in children)
        {
            if(!child.IsActive())
            {
                continue;
            }

            Quest quest = child.getScript<Quest>();
            if (quest != null) {
                quests.Add(quest);
            }
            else {
                Debug.LogWarning("Quest child of object " + gameObject.ToString() + " does not have quest script");
            }
        }

        if (quests != null && quests.Count > 0)
        {
            currentQuest = quests[0];
            questIndex = 0;
        }

        if (currentQuest != null)
        {
            StartCurrentQuest();
        }

        GameObject playerGO = GameObject.FindWithTag("Player");

        if (playerGO != null)
        {
            player = playerGO.getScript<PlayerController_V2>();
            if (player != null)
            {
                player.OnPlayerDeath += HandlePlayerDeath;
            }
        }
    }

    protected override void update()
    {
        currentQuest?.UpdateQuest();
    }
    public void SkipCurrentQuest()
    {
        if (currentQuest == null)
            return;
        do{
            currentQuest.OnSkip();
            questIndex = Mathf.Min(questIndex + 1, quests.Count);
            if (questIndex == quests.Count)
            {
                currentQuest = null;
                break;
            }
            currentQuest = quests[questIndex];
        }
        while (questIndex < quests.Count && !currentQuest.HasCheckpoint());
     
        if(questIndex < quests.Count)
        {
            StartCurrentQuest();
            TeleportToCheckPoint();
        }
       

    }
    private void HandleQuestStateChanged(object sender, Quest.QuestStateChangedEventArgs e)
    {
        if (e.NewState == e.OldState)
        {
            Debug.Log("Quest new/old states same");
            return;
        }

        if (e.NewState == Quest.QuestState.Success)
        {
            currentQuest.OnSuccess();
            MoveToNextQuest();
        }
        else if (e.NewState == Quest.QuestState.Fail)
        {
            currentQuest.OnFail();
        }
    }

    private void MoveToNextQuest()
    {
        currentQuest.OnQuestStateChanged -= HandleQuestStateChanged;
        ++questIndex;
        if (questIndex < quests.Count)
        {
            currentQuest = quests[questIndex];
            StartCurrentQuest();
        }
        else if(questContainer!= null)
        {
            currentQuest = null;
            Debug.Log("Player Won/Quests are done");
            questContainer.SetActive(false);
        }
            
    }

    private void StartCurrentQuest()
    {
        Debug.Log("Quest started");

        currentQuest.OnQuestStateChanged += HandleQuestStateChanged;
        currentQuest.OnEnter();
        if (gameUIManager != null)
            gameUIManager.SetQuestText(currentQuest.GetQuestInformation());
    }

    private void HandlePlayerDeath(object sender, EventArgs e)
    {
        currentQuest?.SetQuestState(Quest.QuestState.Fail);
    }
    public void SkipToQuest(int index)
    {
        for (int i = 0;i<index;++i)
            quests[i].OnSkip();
        questIndex = index;
        currentQuest = quests[questIndex];
        StartCurrentQuest();
        if (questIndex < quests.Count)
            TeleportToCheckPoint();
    }
    public int GetLastCheckpoint()
    {
        if (currentQuest == null)
            return quests.Count - 1;
        while (currentQuest != null && !currentQuest.HasCheckpoint())
        {
            questIndex = Mathf.Max(0, questIndex - 1);
            currentQuest = quests[questIndex];
        }
        return questIndex;
    }
    private void TeleportToCheckPoint()
    {
        player.gameObject.transform.position = currentQuest.GetCheckpointPosition();
        if (playerHead != null && playerOrientation != null)
        {
            float yaw = Mathf.Deg2Rad * currentQuest.GetCheckPointTargetYaw();
            // Currently doing this as euler angles of the checkpoint gameobject somehow changes
            playerHead.transform.localEulerAngles = new Vector3(0, yaw, 0);
            playerOrientation.transform.localEulerAngles = new Vector3(0, yaw, 0);
        }
        player.OnTeleport();
    }
}