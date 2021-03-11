// The state of each square
// Author Josef Olsson (josol381)

using System;
using System.Windows.Media;

namespace CanvasMap
{
    public enum SquareState
    {
        Inside,
        Outside,
        Unknown,
        Start
    }
    public static class SquareStateExtension
    {
        // Used for drawing squares on the map
        public static Brush GetBrush(this SquareState state)
        {
            switch (state)
            {
                case SquareState.Inside:
                    return Brushes.DeepSkyBlue;
                case SquareState.Outside:
                    return Brushes.Red;
                case SquareState.Unknown:
                    return Brushes.Transparent;
                case SquareState.Start:
                    return Brushes.Green;
                default:
                    throw new ArgumentException($"State {state} not implemented");
            }
        }

        public static SquareState CharToState(string c)
        {
            switch (c)
            {
                case "I":
                    return SquareState.Inside;
                case "O":
                    return SquareState.Outside;
                case "U":
                    return SquareState.Unknown;
                case "S":
                    return SquareState.Start;
                default:
                    throw new ArgumentException($"Char {c} is an invalid state.");
            }
        }

    }
}
