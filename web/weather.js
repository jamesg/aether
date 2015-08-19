var DaysView = CollectionView.extend({
    initialize: function() {
        CollectionView.prototype.initialize.apply(this, arguments);
    },
    initializeView: function(view) {
        this.listenTo(
            view,
            'click',
            (function() {
                this.trigger('click', view.model);
            }).bind(this)
        );
    },
    emptyView: StaticView.extend({
        tagName: 'p',
        className: 'advice',
        template: 'Weather forecast data is not available.'
    }),
    view: StaticView.extend({
        tagName: 'div',
        template: '\
        <%if(detailed_available){%>\
            <label>\
                <input <%if(first){%>checked<%}%> type="radio" name="day" value="<%-date%>"></input>\
                <div class="weather-container weather-container-active">\
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
                    <span class="oi" data-glyph="chevron-right" aria-hidden="true"> </span>\
                    </div>\
                </div>\
            </label>\
        <%}else{%>\
            <label>\
                <div class="weather-container weather-container-inactive">\
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
                    <span class="oi" data-glyph="x" aria-hidden="true"> </span>\
                </div>\
            </label>\
        <%}%>\
        ',
        templateParams: function() {
            var params = _.clone(this.model.attributes);
            _.extend(
                params,
                {
                    first: this.model.collection.at(0) == this.model,
                    str: moment(params['date']).format('dddd Do MMMM'),
                    icon: coalesce(
                        weatherIcon(params['forecast_weather_main']),
                        'question-mark'
                    ),
                    forecast_temp_day_c: Number(params['forecast_temp_day']).toFixed(0),
                    forecast_temp_night_c: Number(params['forecast_temp_night']).toFixed(0)
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
            this.trigger('click');
        }
    })
});

var PhasesView = CollectionView.extend({
    initialize: function() {
        CollectionView.prototype.initialize.apply(this, arguments);
    },
    initializeView: function(view) {
        this.listenTo(
            view,
            'click',
            (function() {
                this.trigger('click', view.model);
            }).bind(this)
        );
    },
    emptyView: StaticView.extend({
        tagName: 'p',
        className: 'advice',
        template: 'Weather forecast data is not available.'
    }),
    view: StaticView.extend({
        tagName: 'div',
        template: '\
        <label>\
            <input <%if(first){%>checked<%}%> type="radio" name="phase" value="<%-phase_id%>"></input>\
            <div class="weather-container weather-container-active">\
                <%-phase_desc%><br>\
                <span class="oi" data-glyph="chevron-right" aria-hidden="true"> </span>\
                </div>\
            </div>\
        </label>\
        ',
        templateParams: function() {
            var params = _.clone(this.model.attributes);
            _.extend(
                params,
                {
                    first: this.model.collection.at(0) == this.model
                }
            );
            return params;
        },
        events: {
            click: 'selectDay'
        },
        selectDay: function() {
            this.trigger('click');
        }
    })
});

var SummaryView = StaticView.extend({
    templateParams: function() {
        return {
            maxTemp: Number(
                _.max(this.model.pluck('forecast_main_temp'), Number.parseFloat)
            ).toFixed(1),
            minTemp: Number(
                _.min(this.model.pluck('forecast_main_temp'), Number.parseFloat)
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
    '
});

var WeatherForecastView = StaticView.extend(
    {
        pageTitle: 'Weather',
        template: $('#weatherforecast-template').html(),
        fadeOut: function() {
            this.$el.addClass('invisible');
        },
        fadeIn: function() {
            this.$el.removeClass('invisible');
            console.log('fade in');
        },
        gotoDay: function(date, phase_id) {
            this.fadeOut();
            this.$('h1[name=weather-title]').html(
                'Weather for ' +
                moment(date).format('dddd Do MMMM YYYY')
                );
            this._forecast.fetch({
                // Provide the timezone offset in minutes so that the
                // server will only provide weather for one day at the
                // local timezone.
                url: restUri(
                    'weather/day/' + moment(date).format('YYYY-MM-DD') +
                    '?timezone=' + (new Date).getTimezoneOffset() +
                    ((phase_id != null) ? ('&phase_id=' + phase_id) : '')
                    ),
                success: this._initializeCharts.bind(this)
            });
        },
        initialize: function() {
            StaticView.prototype.initialize.apply(this, arguments);
            StaticView.prototype.render.apply(this);
            this.fadeOut();

            this._chart = new Chart({ id: 'temperaturechart', type: 'line' })

            var location = new Location;
            location.fetch();
            (new StaticView({
                el: this.$('div[name=location]'),
                template: 'Today\'s weather forecast for <%-location_city%> (<%-location_lat%>&deg; N, <%-location_lon%>&deg; E).',
                model: location
            })).render();

            this._forecast = new ForecastCollection;
        },
        chartData: function() {
            return {
                xScale: 'ordinal',
                yScale: 'linear',
                type: 'line',
                main: [
                    {
                        className: '.temperaturechartdata',
                        interpolation: 'linear',
                        type: 'line',
                        data: this._forecast.map(
                            function(point) {
                                return {
                                    x: moment(point.get('forecast_dt'), 'X').format('HH:mm'),
                                    y: Number.parseFloat(point.get('forecast_main_temp'))
                                };
                            }
                        )
                    }
                ],
                comp: [
                    {
                        type: 'symbolVis',
                        className: '.comp.temperaturechartsymbols',
                        data: this._forecast.map(
                            function(point) {
                                return {
                                    x: moment(point.get('forecast_dt'), 'X').format('HH:mm'),
                                    y: Number.parseFloat(point.get('forecast_main_temp')),
                                    symbol: coalesce(
                                        weatherIcon(point.get('forecast_weather_main')),
                                        'question-mark'
                                    )
                                };
                            }
                        )
                    },
                    {
                        className: '.comp.modelchartdata',
                        interpolation: 'linear',
                        type: 'line',
                        data: this._forecast.map(
                            function(point) {
                                return {
                                    x: moment(point.get('forecast_dt'), 'X').format('HH:mm'),
                                    y: Number(point.get('model_temperature'))/* toFixed(1) is broken */
                                };
                            }
                        )
                    },
                    {
                        className: '.comp.sensorchartdata',
                        interpolation: 'linear',
                        type: 'line',
                        data: this._forecast.filter(
                            function(point) {
                                return _.has(point.attributes, 'sensor_temperature');
                            }
                        ).map(
                            function(point) {
                                return {
                                    x: moment(point.get('forecast_dt'), 'X').format('HH:mm'),
                                    y: point.get('sensor_temperature')/* toFixed(1) is broken */
                                };
                            }
                        )
                    }
                ]
            };
        },
        _initializeCharts: function() {
            console.log('init chart');
            // Day summary.
            (new SummaryView({
                el: this.$('div[name=summary]'),
                model: this._forecast
            })).render();

console.log('init', this.chartData());
            this._chart.setData(this.chartData());

            this.fadeIn();
        },
        render: function() {
        }
    }
    );

var WeatherPage = PageView.extend(
    {
        pageTitle: 'Weather',
        template: $('#weatherpage-template').html(),
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this);

            this._weatherForecastView = new WeatherForecastView({
                el: this.$('div[name=forecast]')
            });

            // Assuming there is a detailed forecast for the current day.
            this._weatherForecastView.gotoDay(moment(), null);

            var dailyForecast = new DailyForecastCollection;
            dailyForecast.fetch({
                success: (function() {
                    this._dayModel = dailyForecast.at(0);
                }).bind(this)
            });
            this._daysView = new DaysView({
                el: this.$('div[name=days]'),
                model: dailyForecast
            });
            this._daysView.render();
            this.listenTo(
                this._daysView,
                'click',
                (function(dayModel) {
                    this._dayModel = dayModel;
                    // Detailed data may not be available for dates some time
                    // in the future.
                    if(dayModel.get('detailed_available'))
                        this._weatherForecastView.gotoDay(
                            this._dayModel.get('date'),
                            this._phaseModel.id
                        );
                }).bind(this)
            );

            var phases = new PhaseCollection;
            phases.fetch({
                success: (function() {
                    this._phaseModel = phases.at(0);
                }).bind(this)
            });
            this._phasesView = new PhasesView({
                el: this.$('div[name=phases]'),
                model: phases
            });
            this._phasesView.render();
            this.listenTo(
                this._phasesView,
                'click',
                (function(phase) {
                    this._phaseModel = phase;
                    // put the model on the graph
                    this._weatherForecastView.gotoDay(
                        this._dayModel.get('date'),
                        this._phaseModel.id
                    );
                }).bind(this)
            );
        },
        render: function() {
        }
    }
)
