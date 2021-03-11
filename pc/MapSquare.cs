// A single square on the map
// Author Josef Olsson (josol381)

using System.Windows;
using System.Windows.Media;
using System.Windows.Shapes;

namespace CanvasMap
{
    class MapSquare
    {
        private SquareState state;

        public MapSquare(SquareState s, double scale, int x, int y)
        {
            Rect = new Rectangle
            {
                Height = scale,
                Width = scale
            };

            // Creates both possible lines
            BottomLine.X1 = scale * (x);
            BottomLine.X2 = scale * (x + 1);
            BottomLine.Y1 = scale * (y + 1);
            BottomLine.Y2 = scale * (y + 1);

            RightLine.X1 = scale * (x + 1);
            RightLine.X2 = scale * (x + 1);
            RightLine.Y1 = scale * (y);
            RightLine.Y2 = scale * (y + 1);
            State = s;
        }

        public SquareState State
        {
            get => state;
            set
            {
                state = value;
                Rect.Fill = State.GetBrush();
            }
        }
        public Rectangle Rect { get; }

        public Line BottomLine { get; } = new Line
        {
            Stroke = Brushes.Black,
            HorizontalAlignment = HorizontalAlignment.Left,
            VerticalAlignment = VerticalAlignment.Top,
            StrokeThickness = 0
        };

        public Line RightLine { get; } = new Line
        {
            Stroke = Brushes.Black,
            HorizontalAlignment = HorizontalAlignment.Left,
            VerticalAlignment = VerticalAlignment.Top,
            StrokeThickness = 0
        };

        // Shows or hides the lines based on the neighbouring squares
        public void UpdateBottomLine(MapSquare bottomSquare) =>
            BottomLine.StrokeThickness = NeedsLine(bottomSquare.State) ? 2 : 0;

        public void UpdateRightLine(MapSquare rightSquare) =>
            RightLine.StrokeThickness = NeedsLine(rightSquare.State) ? 2 : 0;

        // Returns whether a line should be drawn between teo squares
        private bool NeedsLine(SquareState square)
        {
            return (State == SquareState.Inside && square == SquareState.Outside) ||
                   (State == SquareState.Outside && square == SquareState.Inside) ||
                   (State == SquareState.Start && square == SquareState.Outside) ||
                   (State == SquareState.Outside && square == SquareState.Start);
            // Expression above could handle other SquareStates better than comment below
            //return State != square && State != SquareState.Unknown && square != SquareState.Unknown;
        }
    }
}
