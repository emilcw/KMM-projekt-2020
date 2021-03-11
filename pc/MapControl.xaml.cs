using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;

namespace CanvasMap
{
    /// <summary>
    /// Interaction logic for MapControl.xaml
    /// Author Josef Olsson (josol381)
    /// </summary>
    public partial class MapControl : UserControl
    {
        private readonly MapSquare[,] s = new MapSquare[MAP_SIZE_X, MAP_SIZE_Y];
        private const int PADDING = 1;
        private const int MAP_SIZE_X = PADDING*2 + 49;
        private const int MAP_SIZE_Y = PADDING*2 + 49;
        private double Scale { get => System.Math.Min(ActualHeight, ActualWidth) / 51; }
        private Robot robot;
        private TranslateTransform targetTransform = new TranslateTransform();
        private readonly Line targetLine = new Line { Stroke = Brushes.Black, StrokeThickness = 2};
        public MapControl()
        {
            InitializeComponent();
        }

        // Init must be called after the window is created or else the
        // actual height and width of the element is 0
        public void Init(object sender = null, System.Windows.RoutedEventArgs e = null)
        {
            // Initiates all squares to unknown
            for (int x = 0; x < MAP_SIZE_X; x++)
            {
                for (int y = 0; y < MAP_SIZE_Y; y++)
                {
                    InitSquare(x, y, SquareState.Unknown);
                }
            }

            // Initiates the robot
            robot = new Robot(Scale, MAP_SIZE_Y);
            RobotLayer.Children.Add(robot.R);
            SetRobotPos(MAP_SIZE_X / (double)2, MAP_SIZE_Y / (double)2);
            SetRobotAngle(0);

            // Initiates the targeting
            Ellipse target = new Ellipse
            {
                Height = Scale,
                Width = Scale,
                Stroke = Brushes.Black,
                StrokeThickness = 4
            };
            target.RenderTransform = targetTransform;
            RobotLayer.Children.Add(target);
            SetTarget(MAP_SIZE_X / (double)2, MAP_SIZE_Y / (double)2);

            RobotLayer.Children.Add(targetLine);
        }

        // Initiates a square
        private void InitSquare(int x, int y, SquareState state)
        {
            // Create new square
            s[x, y] = new MapSquare(state, Scale, x, y);

            // Draw rectangle
            Rectangle r = s[x, y].Rect;
            CM.Children.Add(r);
            Canvas.SetTop(r, Scale * y);
            Canvas.SetLeft(r, Scale * x);
            
            CM.Children.Add(s[x, y].BottomLine);
            CM.Children.Add(s[x, y].RightLine);
        }

        // Sets all initiated squares
        public void SetAllSquares(SquareState state)
        {
            for (int x = 0; x < MAP_SIZE_X; x++)
            {
                for (int y = 0; y < MAP_SIZE_Y; y++)
                {
                    SetSquare(x, y, state);
                }
            }
        }

        // Sets an initiated square and updates possible walls
        // (0,0) is the bottom left corner of the possible map
        public void SetSquare(int x, int y, SquareState state)
        {
            // Adapts inputs to the grid
            y = MAP_SIZE_Y - y - 1;

            s[x, y].State = state;
            // Get new lines
            if (x > 0) s[x - 1, y].UpdateRightLine(s[x, y]);
            if (x < MAP_SIZE_X - 1) s[x, y].UpdateRightLine(s[x + 1, y]);
            if (y > 0) s[x, y - 1].UpdateBottomLine(s[x, y]);
            if (y < MAP_SIZE_Y - 1) s[x, y].UpdateBottomLine(s[x, y + 1]);
        }

        // Sets the current position of the robot and updates
        // the target line to originate from the new pos
        public void SetRobotPos(double x, double y)
        {
            robot.SetPos(x, y);
            targetLine.X1 = x * Scale;
            targetLine.Y1 = (MAP_SIZE_Y - y) * Scale;
        }

        // Sets the current angle of the robot
        public void SetRobotAngle(double a) => robot.SetAngle(a);

        // Sets the target to the specified position and updates
        // lines accordingly
        public void SetTarget(double x, double y)
        {
            targetLine.X2 = (x + 0.5) * Scale;
            targetLine.Y2 = (MAP_SIZE_Y - y - 0.5) * Scale;

            y = MAP_SIZE_Y - y;

            targetTransform.X = x * Scale;
            targetTransform.Y = (y - 1) * Scale;
        }
    }
}
