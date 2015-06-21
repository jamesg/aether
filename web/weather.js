var WeatherPage = PageView.extend(
        {
            pageTitle: 'Weather',
            template: $('#weatherpage-template').html(),
            initialize: function() {
                PageView.prototype.initialize.apply(this, arguments);
                PageView.prototype.render.apply(this);
                this.initializeCharts();
            },
            initializeCharts: function() {
                var points = new ForecastCollection;
                points.fetch({
                    url: restUri(
                             'weather/today?timezone=' +
                             (new Date).getTimezoneOffset()
                             ),
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
                        var temperatureChartData = {
                            xScale: 'ordinal',
                            yScale: 'linear',
                            type: 'line',
                            main: [
                                {
                                    className: '.temperaturechartdata',
                                    interpolation: 'linear',
                                    data: temperatureSeries
                                }
                            ]
                        };
                        var rainChartData = {
                            xScale: 'ordinal',
                            yScale: 'linear',
                            type: 'bar',
                            main: [
                                {
                                    className: '.temperaturechartdata',
                                    interpolation: 'linear',
                                    data: rainSeries
                                }
                            ]
                        };
                        if(_.has(this, '_temperatureChart'))
                            this._temperatureChart.setData(temperatureChartData);
                        else
                            this._temperatureChart =
                                new xChart('bar', temperatureChartData, '#temperaturechart');
                        if(_.has(this, '_rainChart'))
                            this._rainChart.setData(rainChartData);
                        else
                            this._rainChart =
                                new xChart('bar', rainChartData, '#rainchart');
                    }).bind(this)
                });
            },
            render: function() {
            }
        }
        );

