using System;
using System.Collections.Generic;
using System.Net;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using Wpf.CartesianChart.ConstantChanges;

namespace PCInterface
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// Author Josef Olsson (josol381)
    /// </summary>
    public partial class MainWindow : Window
    {
        private readonly TcpClient tcp = new TcpClient();
        private bool isReading = false;
        private readonly bool debug;
        private readonly Dictionary<string, ConstantChangesChart> charts; // Useful if multiple graphs were to be added
        private bool manualMode = false;
        private static readonly System.Globalization.CultureInfo useDotNotComma = new System.Globalization.CultureInfo("en-US");

        public MainWindow(IPAddress ip, bool debug)
        {
            if (ip == null) throw new ArgumentNullException("ip should not be null");
            
            InitializeComponent();
            DataContext = this;
            this.debug = debug;

            SetTcpConnectedVisibility(Visibility.Hidden);
            
            IPAddrBox.Text = ip.ToString();
            EstablishConnection();
                
            version.Text = Assembly.GetExecutingAssembly().GetName().Version.ToString();
            debugMode.Visibility = debug ? Visibility.Visible : Visibility.Hidden;

            charts = new Dictionary<string, ConstantChangesChart> { { "speed", SpeedChart }, {"angle", AngleChart } };
            SpeedChart.AddSeries("motor1", Brushes.Red);
            SpeedChart.AddSeries("motor2", Brushes.Blue);
            AngleChart.AddSeries("lidarangle", Brushes.Green);
            AngleChart.AddSeries("gyro", Brushes.Yellow);

            // Has to be executed on another thread for the actual size to be properly initiated
            Task.Factory.StartNew(InitMap);
        }

        private void InitMap()
        {
            Dispatcher.Invoke(() => { Map.Init(); });
            Thread.Yield();
        }

        /*****************
         * Tcp functions *
         *****************/
        // Sets visibility of some elements related to being connected via tcp
        private void SetTcpConnectedVisibility(Visibility v)
        {
            //ToggleGatheringButton.Visibility = v;
            //ButtonGrid.Visibility = v;
            MoveGrid.Visibility = manualMode ? v : Visibility.Hidden;
            StartButton.Visibility = debug ? Visibility.Visible : Visibility.Hidden;
            SendDataGrid.Visibility = debug ? v : Visibility.Hidden;
        }

        // Tries to establish a tcp connection
        private void EstablishConnection(object sender = null, RoutedEventArgs e = null)
        {
            // Tests if the address is a valid IP
            if (IPAddress.TryParse(IPAddrBox.Text, out IPAddress ip))
            {
                // Tries to connect to the ip
                if (tcp.ConnectToServer(ip.ToString()))
                {
                    IPAddrBox.Foreground = Brushes.Green;
                    SetTcpConnectedVisibility(Visibility.Visible);
                    ToggleDataGathering();
                    DataBox.Focus();
                    ConnectionButton.Visibility = Visibility.Hidden;
                    IPAddrBox.Focusable = false;
                }
                else
                {
                    IPAddrBox.Foreground = Brushes.Red;
                    MessageBox.Show("Could not establish connection", "Connection error", MessageBoxButton.OK, MessageBoxImage.Error);
                    IPAddrBox.Focus();
                    IPAddrBox.CaretIndex = IPAddrBox.Text.Length;
                }
            }
            else
            {
                IPAddrBox.Foreground = Brushes.Red;
                MessageBox.Show("Invalid IP address", "Invalid IP address", MessageBoxButton.OK, MessageBoxImage.Error);
                IPAddrBox.Focus();
                IPAddrBox.CaretIndex = IPAddrBox.Text.Length;
            }
        }

        // Barely used anymore but kept for the moment
        private void ToggleDataGathering(object sender = null, RoutedEventArgs e = null)
        {
            isReading = !isReading;
            if (isReading) Task.Factory.StartNew(RecieveTCPData);
            ToggleGatheringButton.Content = (string)ToggleGatheringButton.Content == "Start gathering data" ? "Stop gathering data" : "Start gathering data";
        }

        // Continuously updates the graph and recieved tcp messages
        private void RecieveTCPData()
        {
            while (isReading)
            {
                Thread.Sleep(50);

                string[][] message = tcp.Recive();
                var now = DateTime.Now;
                if (message != null)
                {
                    // Dispatcher solution:
                    //      Dispatcher.Invoke(() => { });
                    // was found at https://stackoverflow.com/questions/9732709/the-calling-thread-cannot-access-this-object-because-a-different-thread-owns-it
                    // and is needed for the correct thread to do the correct task
                    foreach (var command in message)
                    {
                        // command[0] is which command, command[1, 2, ...] is arguments to the command
                        switch (command[0].ToLower())
                        {
                            case "mode":
                                Dispatcher.Invoke(() =>
                                {
                                    Mode.Text = command[1];
                                    manualMode = Mode.Text == "manual";
                                    MoveGrid.Visibility = manualMode ? Visibility.Visible : Visibility.Hidden;
                                    StartButton.Visibility = manualMode ? Visibility.Hidden : Visibility.Visible;
                                });
                                break;
                            case "status":
                                Dispatcher.Invoke(() => Status.Text = command[1]);
                                break;
                            case "gyro":
                                Dispatcher.Invoke(() =>
                                {
                                    Gyro.Text = command[1];
                                    Map.SetRobotAngle(parseDouble(command[1]));
                                    charts["angle"].AddToGraph("gyro", now, parseDouble(command[2]));
                                });
                                break;
                            case "lidarspeed":
                                Dispatcher.Invoke(() =>
                                {
                                    Map.SetRobotAngle(parseDouble(command[1]));
                                    charts["angle"].AddToGraph("lidarangle", now, parseDouble(command[2]));
                                });
                                break;
                            case "move":
                                Dispatcher.Invoke(() => Map.SetTarget(parseDouble(command[1]), parseDouble(command[2])));
                                break;
                            case "position":
                                Dispatcher.Invoke(() => Map.SetRobotPos(parseDouble(command[1]), parseDouble(command[2])));
                                break;
                            case "motorspeed":
                                Dispatcher.Invoke(() => charts["speed"].AddToGraph(command[1], now, parseDouble(command[2])));
                                break;
                            case "map":
                                Dispatcher.Invoke(() => Map.SetSquare(int.Parse(command[1]), int.Parse(command[2]),
                                    CanvasMap.SquareStateExtension.CharToState(command[3])));
                                break;
                            case "":
                                break;
                            default:
                                break;
                        }
                    }
                }

                // Updates all charts (currently only one)
                foreach (var chart in charts)
                {
                    chart.Value.UpdateChart(now);
                }
            }
            Thread.Yield();

            // The parameter useDotNotComma is used to force the parsing to use a dot
            // to indicate the decimals and not a comma ie 3.14 instead of 3,14.
            double parseDouble(string command) => double.Parse(command, useDotNotComma);
        }

        /****************************
         * XAML-connected functions *
         ****************************/

        /*
         * Sends moves from button presses.
         */
        private void SendMove(Direction dir) => tcp.Send(dir + ";");

        /*
         * Button presses.
         */
        private void LeftForward(object sender, RoutedEventArgs e) => SendMove(Direction.LeftForward);
        private void Forward(object sender, RoutedEventArgs e) => SendMove(Direction.Forward);
        private void RightForward(object sender, RoutedEventArgs e) => SendMove(Direction.RightForward);
        private void TurnLeft(object sender, RoutedEventArgs e) => SendMove(Direction.TurnLeft);
        private void Stay(object sender, RoutedEventArgs e) => SendMove(Direction.Stay);
        private void TurnRight(object sender, RoutedEventArgs e) => SendMove(Direction.TurnRight);
        private void Backward(object sender, RoutedEventArgs e) => SendMove(Direction.Backward);

        // Initiates the auto exploration, used only for debugging
        private void StartExploration(object sender, RoutedEventArgs e)
        {
            if (Mode.Text == "Exploration complete!" || true)
            {
                Map.SetAllSquares(CanvasMap.SquareState.Unknown);
                tcp.Send("start;");
            }
        }

        /*
         * Text box interactions
         */
        // Allows the user to input a specific IPAddress
        private void IPAddrBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter) EstablishConnection();
        }

        private void SendData(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                tcp.Send(DataBox.Text + ";");
                DataBox.Text = "";
            }
        }

        // Detects when a move-command should be sent
        private void ManualMoveBox_TextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
        {
            switch (manualMoveBox.Text.ToLower())
            {
                case "a":
                    SendMove(Direction.TurnLeft);
                    break;
                case "q":
                    SendMove(Direction.LeftForward);
                    break;
                case "w":
                    SendMove(Direction.Forward);
                    break;
                case "e":
                    SendMove(Direction.RightForward);
                    break;
                case "d":
                    SendMove(Direction.TurnRight);
                    break;
                case "s":
                    SendMove(Direction.Backward);
                    break;
                case " ":
                    SendMove(Direction.Stay);
                    break;
                default:
                    break;
            }
            manualMoveBox.Text = "";
        }
    }
}
