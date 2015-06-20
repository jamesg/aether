var WeatherPage = PageView.extend(
        {
            pageTitle: 'Weather',
            template: $('#weatherpage-template').html(),
            initialize: function() {
                PageView.prototype.initialize.apply(this, arguments);
                PageView.prototype.render.apply(this);
                this.initializeChart();
            },
            initializeChart: function() {
                var points = new ForecastCollection;
                points.fetch({
                    url: restUri('weather/today'),
                    success: (function() {
                        var temperatureSeries = points.map(
                            function(point) {
                                return {
                                    x: moment(point.get('forecast_dt'), 'X').format('HH:mm'),
                                    y: Number((point.get('forecast_main_temp') - 273.15).toFixed(1))
                                };
                            }
                            );
                        var rainSeries = points.map(
                            function(point) {
                                return {
                                    x: moment(point.get('forecast_dt'), 'X').format('HH:mm'),
                                    y: Number(point.get('forecast_rain'))
                                };
                            }
                            );
                        console.log('temperature', JSON.stringify(temperatureSeries));
                        console.log('rain', JSON.stringify(rainSeries));
                        var chartData = {
                            xScale: 'ordinal',
                            yScale: 'linear',
                            type: 'bar',
                            main: [
                                {
                                    className: '.rainchartdata',
                                    type: 'bar',
                                    data: rainSeries
                                }
                            ],
                            comp: [
                                {
                                    className: '.temperaturechartdata',
                                    interpolation: 'linear',
                                    type: 'line',
                                    data: temperatureSeries
                                }
                            ]
                        };
                        try {
                            if(_.has(this, '_chart'))
                                this._chart.setData(chartData);
                            else
                                this._chart =
                                    new xChart('bar', chartData, '#weatherchart');
                        } catch(err) {
                            console.log('caught', err);
                        }
                    }).bind(this)
                });
            },
            render: function() {
            }
        }
        );

