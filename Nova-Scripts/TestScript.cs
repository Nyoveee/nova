using ScriptingAPI;

namespace Nova_Scripts
{
    public class TestScript : Script
    {
        public override void Init()
        {
            Console.WriteLine("Init");
        }
        public override void update()
        {
            Console.WriteLine("Update");
        }  
        public override void Exit()
        {
            Console.WriteLine("Exit");

        }
    }
}
