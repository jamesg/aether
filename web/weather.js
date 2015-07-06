var kelvinToCelsius = function(degK) {
    return degK - 273.15;
};

// Map of forecast_weather_main content to an appropriate icon.
var weatherIcon = {
    Clear: 'sun',
    Clouds: 'cloud',
    Rain: 'rain'
};

// Visualisation type for weather symbols.
var symbolVis = {
    enter: function(self, storage, className, data, callbacks) {
        // Select the highest z-index.
        var insertionPoint = xChart.visutils.getInsertionPoint(9);

        storage.container = self._g.selectAll('.temperaturechartsymbols' + className)
            .data(
                data,
                function(d) {
                    return d.className;
                }
            );
        storage.container.enter().insert('g', insertionPoint).attr(
            'class',
            function (d, i) {
                return 'temperaturechartsymbols' + className.replace(/\./g, ' ') +
                    ' color' + i;
            }
        );
        storage.symbols = storage.container.selectAll('g').data(
            function (d) {
                return d.data;
            },
            function (d) {
                return d.x;
            }
        );

        // Create a group for each symbol to translate it independently of
        // scaling.
        storage.symbols.enter()
            .insert('g')
            .attr(
                'transform',
                function(d) {
                    return 'translate(' +
                        (self.xScale(d.x) + self.xScale.rangeBand() / 2) + ' ' +
                        self.yScale(d.y) +
                        ')';
                }
            )
            .insert('use')
            // Use an Open Iconic icon.
            .attr('xlink:href', function(d) { return '#' + d['symbol']; })
            // Centre the symbol on (0, 0) and scale it.
            .attr('transform', 'scale(5) translate(-4 -4)');
    },
    update: function(self, storage, timing) {
        storage.symbols.transition().duration(timing)
            .style('opacity', 1)
            .attr(
                'transform',
                function(d) {
                    // self.xScale and self.yScale will have changed.
                    return 'translate(' +
                        (self.xScale(d.x) + self.xScale.rangeBand() / 2) + ' ' +
                        self.yScale(d.y) +
                        ')';
                }
            );
    },
    exit: function(self, storage, timing) {
        storage.symbols.exit()
            .transition().duration(timing)
            .style('opacity', 0);
    },
    destroy: function(self, storage, timing) {
        storage.symbols.transition().duration(timing)
            .style('opacity', 0)
            .remove();
    }
};
xChart.setVis('symbolVis', symbolVis);

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
                    <div class="\
                        weather-container\
                        weather-container-<%if(detailed_available){%>active<%}else{%>inactive<%}%>\
                        ">\
                    <%-str%><br>\
                    <div class="weather-temperature-container">\
                        <div class="weather-temperature">\
                            <span>\
                                <span class="oi" data-glyph="sun" aria-hidden="true"> </span>\
                                <%-forecast_temp_day_c%>&deg;C\
                            </span>\
                            <br>\
                            <span>\
                                <span class="oi" data-glyph="moon" aria-hidden="true"> </span>\
                                <%-forecast_temp_night_c%>&deg;C\
                            </span>\
                        </div>\
                        <div class="weather-icon">\
                            <span class="oi" data-glyph="<%-icon%>" aria-hidden="true"> </span>\
                        </div>\
                    </div>\
                    <br>\
                    <%if(detailed_available){%>\
                        <span class="oi" data-glyph="chevron-right" aria-hidden="true"> </span>\
                    <%}else{%>\
                        <span class="oi" data-glyph="x" aria-hidden="true"> </span>\
                    <%}%>\
                    </div>\
                    ',
                    templateParams: function() {
                        var params = _.clone(this.model.attributes);
                        _.extend(
                            params,
                            {
                                str: moment(params['date']).format('dddd Do MMMM'),
                                icon: coalesce(
                                    weatherIcon[params['forecast_weather_main']],
                                    'question-mark'
                                ),
                                forecast_temp_day_c: kelvinToCelsius(params['forecast_temp_day']).toFixed(0),
                                forecast_temp_night_c: kelvinToCelsius(params['forecast_temp_night']).toFixed(0)
                            }
                        );
                        if(params.icon == 'question-mark')
                            console.log('unknown weather type', params['forecast_weather_main'])
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
                <div class="weather-temperature-container weather-temperature-container-large">\
                    <div class="weather-temperature">\
                        <h3>Maximum Daytime Temperature</h3>\
                        <span>\
                            <span class="oi" data-glyph="sun" aria-hidden="true"> </span>\
                            <%-maxTemp%>&deg;C\
                        </span>\
                        <h3>Minimum Nighttime Temperature</h3>\
                        <span>\
                            <span class="oi" data-glyph="moon" aria-hidden="true"> </span>\
                            <%-minTemp%>&deg;C\
                        </span>\
                    </div>\
                </div>\
                ',
                model: this._forecast
            })).render();
            var temperatureSeries = this._forecast.map(
                function(point) {
                    return {
                        x: moment(point.get('forecast_dt'), 'X').format('HH:mm'),
                        y: Number(kelvinToCelsius(point.get('forecast_main_temp')).toFixed(1)),
                        symbol: coalesce(
                            weatherIcon[point.get('forecast_weather_main')],
                            'question-mark'
                        )
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
                ],
                comp: [
                    {
                        type: 'symbolVis',
                        className: '.comp.temperaturechartsymbols',
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
