using System.Net;
using System.Windows;

namespace PCInterface
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// Author Josef Olsson (josol381)
    /// </summary>
    public partial class App : Application
    {
        private void Application_Startup(object sender, StartupEventArgs e)
        {
            IPAddress ip = null; ;
            bool ignoreArgs = false;
            bool debug = false;
            // Commandline arguments
            foreach (var arg in e.Args)
            {
                // An IPAddress can be provided
                if (IPAddress.TryParse(arg, out IPAddress _ip)) ip = _ip;
                // "I" Ignores the IP
                if (arg == "I")
                {
                    ignoreArgs = true;
                    ip = null;
                    break;
                }
                // Used to set debug mode for development and testing
                if (arg == "debug") debug = true;
            }
            ip = ip ?? IPAddress.Parse("192.168.4.1");
            MainWindow main = ignoreArgs ? new MainWindow(ip, debug) : new MainWindow(ip, debug);
            main.Show();
        }
    }
}
