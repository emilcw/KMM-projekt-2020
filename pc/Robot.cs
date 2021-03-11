// The robot drawn on the map
// Author Josef Olsson (josol381)

using System.Windows.Media;
using System.Windows.Shapes;

namespace CanvasMap
{
    class Robot
    {
        public Rectangle R { get; set; }
        private readonly TranslateTransform translate = new TranslateTransform();
        private readonly RotateTransform rotate = new RotateTransform();
        private readonly TransformGroup transform = new TransformGroup();
        private readonly double scale;
        private readonly int MAP_SIZE_Y;
        private const double LENGTH = 0.8;
        private const double WIDTH = 0.5;

        public Robot(double scale, int MAP_SIZE_Y)
        {
            this.scale = scale;
            this.MAP_SIZE_Y = MAP_SIZE_Y;
            R = new Rectangle
            {
                Height = scale * LENGTH,
                Width = scale * WIDTH,
                Fill = Brushes.Gray
            };

            transform.Children.Add(rotate);
            transform.Children.Add(translate);
            R.RenderTransform = transform;
            R.RenderTransformOrigin = new System.Windows.Point(0.5,0.5);
        }

        public void SetPos(double x, double y)
        {
            x -= WIDTH / 2;
            y = MAP_SIZE_Y - y - LENGTH / 2;

            translate.X = x * scale;
            translate.Y = y * scale;
        }

        public void SetAngle(double a) => rotate.Angle = 90 - a;
    }
}
