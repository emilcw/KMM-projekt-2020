// Most of this file was copied from https://lvcharts.net/App/examples/v1/wpf/Constant%20Changes
// and some from https://lvcharts.net/App/examples/v1/wpf/Basic%20Line%20Chart
// Author Josef Olsson (josol381)

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows.Controls;
using System.Windows.Media;
using LiveCharts;
using LiveCharts.Configurations;
using LiveCharts.Wpf;

namespace Wpf.CartesianChart.ConstantChanges
{
    public partial class ConstantChangesChart : UserControl, INotifyPropertyChanged
    {
        private double _axisMax;
        private double _axisMin;

        public ConstantChangesChart()
        {
            InitializeComponent();

            //To handle live data easily, in this case we built a specialized type
            //the MeasureModel class, it only contains 2 properties
            //DateTime and Value
            //We need to configure LiveCharts to handle MeasureModel class
            //The next code configures MeasureModel  globally, this means
            //that LiveCharts learns to plot MeasureModel and will use this config every time
            //a IChartValues instance uses this type.
            //this code ideally should only run once
            //you can configure series in many ways, learn more at 
            //http://lvcharts.net/App/examples/v1/wpf/Types%20and%20Configuration

            var mapper = Mappers.Xy<MeasureModel>()
                .X(model => model.DateTime.Ticks)   //use DateTime.Ticks as X
                .Y(model => model.Value);           //use the value property as Y

            //lets save the mapper globally.
            Charting.For<MeasureModel>(mapper);

            //lets set how to display the X Labels
            DateTimeFormatter = value => new DateTime((long)value).ToString("hh:mm:ss");

            //AxisStep forces the distance between each separator in the X axis
            AxisStep = TimeSpan.FromSeconds(1).Ticks;
            //AxisUnit forces lets the axis know that we are plotting seconds
            //this is not always necessary, but it can prevent wrong labeling
            AxisUnit = TimeSpan.TicksPerSecond;

            SetAxisLimits(DateTime.Now);

            DataContext = this;
        }

        public SeriesCollection SeriesCollection { get; set; } = new SeriesCollection();
        private Dictionary<string, List<MeasureModel>> DatapointsToAdd = new Dictionary<string, List<MeasureModel>>();

        public Func<double, string> DateTimeFormatter { get; set; }
        public double AxisStep { get; set; }
        public double AxisUnit { get; set; }
        public double AxisMax
        {
            get { return _axisMax; }
            set
            {
                _axisMax = value;
                OnPropertyChanged("AxisMax");
            }
        }
        public double AxisMin
        {
            get { return _axisMin; }
            set
            {
                _axisMin = value;
                OnPropertyChanged("AxisMin");
            }
        }

        // Self written func
        public void AddToGraph(string title, DateTime now, double data)
            => DatapointsToAdd[title].Add(new MeasureModel { DateTime = now, Value = data });

        // Modified Read()
        public void UpdateChart(DateTime now)
        {
            foreach (var series in SeriesCollection)
            {
                IChartValues values = series.Values;
                string title = "";
                // Dispatcher solution:
                //      Dispatcher.Invoke(() => { });
                // was found at https://stackoverflow.com/questions/9732709/the-calling-thread-cannot-access-this-object-because-a-different-thread-owns-it
                Dispatcher.Invoke(() => title = series.Title );

                values.AddRange(DatapointsToAdd[title]);
                DatapointsToAdd[title] = new List<MeasureModel>();

                SetAxisLimits(now);

                //lets only use the last 150 values
                if (values.Count > 150) values.RemoveAt(0);
            }
        }

        public void AddSeries(string title, Brush b)
        {
            SeriesCollection.Add(new LineSeries
            {
                Title = title,
                LineSmoothness = 0.3, //0: straight lines, 1: really smooth lines
                Values = new ChartValues<MeasureModel>(),
                PointGeometry = null,
                StrokeThickness = 3,
                Stroke = b,
                Fill = Brushes.Transparent
            });
            DatapointsToAdd[title] = new List<MeasureModel>();
        }

        private void SetAxisLimits(DateTime now)
        {
            AxisMax = now.Ticks + TimeSpan.FromSeconds(1).Ticks; // lets force the axis to be 1 second ahead
            AxisMin = now.Ticks - TimeSpan.FromSeconds(8).Ticks; // and 8 seconds behind
        }
        #region INotifyPropertyChanged implementation

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName = null)
        {
            if (PropertyChanged != null)
                PropertyChanged.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        #endregion
    }
}