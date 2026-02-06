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
    protected override void init()
    {
        gameUIManager = GameObject.FindWithTag("Game UI Manager")?.getScript<GameUIManager>();
        GameObject[] children = gameObject.GetChildren();

        foreach (var child in children)
        {
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

        if (gameUIManager != null)
        {
            gameUIManager.RestartFromCheckpointButton += RestartQuest;
        }
    }

    protected override void update()
    {
        if (currentQuest != null)
            currentQuest?.UpdateQuest();
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
            currentQuest.OnFail(player.gameObject.transform);
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
        if (gameUIManager != null)
        {
            currentQuest.SetQuestState(Quest.QuestState.Fail);
            gameUIManager.TriggerDeathScreen(); 
            if (player != null)
            {
                player.playerMoveStates = PlayerMoveStates.Disabled;
            }
            // Disable player keys
        }
    }

    private void RestartQuest()
    {
        if (currentQuest != null)
            currentQuest.SetQuestState(Quest.QuestState.InProgress);
        if (player != null)
        {
            player.gameObject.transform.position = currentQuest.GetCheckpointPosition();
            player.playerMoveStates = PlayerMoveStates.GroundedMovement;
            player.Reset();
            foreach (GameObject hitbox in GameObject.FindGameObjectsWithTag("EnemyHitBox"))
                Destroy(hitbox);
        }
    }
}