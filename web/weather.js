var kelvinToCelsius = function(degK) {
    return degK - 273.15;
};

var DailyForecast = RestModel.extend(
    {
        idAttribute: 'forecast_dt',
        defaults: {
            date: '',
            detailed_available: false,
            forecast_weather_main: '',
            forecast_weather_description: '',
            forecast_temp_day: 0,
            forecast_temp_night: 0,
            forecast_wind_speed: 0,
            forecast_wind_deg: 0,
            forecast_clouds: 0
        }
    }
);

var DailyForecastCollection = RestCollection.extend(
    {
        model: DailyForecast,
        url: restUri('weather/day')
    }
);

var WeatherPage = PageView.extend(
    {
        pageTitle: 'Weather',
        template: $('#weatherpage-template').html(),
        gotoDay: function(date) {
            this.$('h1[name=weather-title]').html(
                'Weather for ' +
                moment(date).format('dddd Do MMMM YYYY')
                );
            this._forecast.fetch({
                // Provide the timezone offset in minutes so that the
                // server will only provide weather for one day at the
                // local timezone.
                url: restUri(
                    'weather/day/' + date + '?timezone=' +
                    (new Date).getTimezoneOffset()
                    ),
                success: this._initializeCharts.bind(this)
            });
        },
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this);

            var location = new Location;
            location.fetch();
            (new StaticView({
                el: this.$('div[name=location]'),
                template: 'Today\'s weather forecast for <%-location_city%> (<%-location_lat%>&deg; N, <%-location_lon%>&deg; E).',
                model: location
            })).render();

            this._forecast = new ForecastCollection;
            // Assuming there is a detailed forecast for the current day.
            this.gotoDay(moment().format('YYYY-MM-DD'));

            var page = this;

            var dailyForecast = new DailyForecastCollection;
            dailyForecast.fetch();
            (new CollectionView({
                el: this.$('div[name=days]'),
                view: StaticView.extend({
                    tagName: 'div',
                    template: '\
                    <div class="weather-container">\
                    <%-str%><br>\
                    <span class="oi weather-icon" data-glyph="<%-icon%>" aria-hidden="true"> </span>\
                    <%if(detailed_available){%>\
                        <br>\
                        <span class="oi" data-glyph="chevron-right" aria-hidden="true"> </span>\
                    <%}else{%>&nbsp;<%}%>\
                    </div>\
                    ',
                    templateParams: function() {
                        var params = _.clone(this.model.attributes);
                        _.extend(
                            params,
                            {
                                str: moment(params['date']).format('dddd Do MMMM'),
                                icon: coalesce(
                                    {
                                        Clear: 'sun',
                                        Rain: 'rain'
                                    }[params['forecast_weather_main']],
                                    'question-mark'
                                )
                            }
                        );
                        return params;
                    },
                    events: {
                        click: 'selectDay'
                    },
                    selectDay: function() {
                        // Detailed data may not be available for dates some
                        // time in the future.
                        if(this.model.get('detailed_available'))
                            page.gotoDay(this.model.get('date'));
                    }
                }),
                model: dailyForecast
            }));
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
