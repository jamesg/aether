var kelvinToCelsius = function(degK) {
    return degK - 273.15;
};

var WeatherPage = PageView.extend(
        {
            pageTitle: 'Weather',
            template: $('#weatherpage-template').html(),
            initialize: function() {
                PageView.prototype.initialize.apply(this, arguments);
                PageView.prototype.render.apply(this);

                this._forecast = new ForecastCollection;
                this._forecast.fetch({
                    // Provide the timezone offset in minutes so that the
                    // server will only provide weather for one day at the
                    // local timezone.
                    url: restUri(
                             'weather/today?timezone=' +
                             (new Date).getTimezoneOffset()
                             ),
                    success: this._initializeCharts.bind(this)
                });

                var location = new Location;
                location.fetch();

                (new StaticView({
                    el: this.$('div[name=location]'),
                    template: 'Today\'s weather forecast for <%-location_city%> (<%-location_lat%>&deg; N, <%-location_lon%>&deg; E).',
                    model: location
                })).render();
            },
            _initializeCharts: function() {
                (new StaticView({
                    el: this.$('div[name=summary]'),
                    templateParams: function() {
                        return {
                            maxTemp: kelvinToCelsius(
                                    _.max(this.model.pluck('forecast_main_temp'))
                                    ).toFixed(1),
                            minTemp: kelvinToCelsius(
                                    _.min(this.model.pluck('forecast_main_temp'))
                                    ).toFixed(1)
                        };
                    },
                    template: '\
                    <h2>Summary</h2>\
                    Min: <%-minTemp%> &deg;C, max: <%-maxTemp%> &deg;C\
                    ',
                    model: this._forecast
                })).render();
                var temperatureSeries = this._forecast.map(
                    function(point) {
                        return {
                            x: moment(point.get('forecast_dt'), 'X').format('HH:mm'),
                            y: Number(kelvinToCelsius(point.get('forecast_main_temp')).toFixed(1))
                        };
                    }
                    );
                var rainSeries = this._forecast.map(
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
                        new xChart('line', temperatureChartData, '#temperaturechart');
                if(_.has(this, '_rainChart'))
                    this._rainChart.setData(rainChartData);
                else
                    this._rainChart =
                        new xChart('bar', rainChartData, '#rainchart');
            },
            render: function() {
            }
        }
        );

