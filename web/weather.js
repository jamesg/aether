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

var SummaryView = StaticView.extend({
    templateParams: function() {
        return {
            maxTemp: Number(_.max(this.model.pluck('forecast_main_temp'))).toFixed(1),
            minTemp: Number(_.min(this.model.pluck('forecast_main_temp'))).toFixed(1)
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

            this._chart = new Chart({ id: 'temperaturechart', type: 'line' })

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

            var dailyForecast = new DailyForecastCollection;
            dailyForecast.fetch();
            this._daysView = new DaysView({
                el: this.$('div[name=days]'),
                model: dailyForecast
            });
            this._daysView.render();
            this.listenTo(
                this._daysView,
                'click',
                (function(dayModel) {
                    // Detailed data may not be available for dates some time
                    // in the future.
                    if(dayModel.get('detailed_available'))
                        this.gotoDay(dayModel.get('date'));
                }).bind(this)
            );
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
                                        weatherIcon[point.get('forecast_weather_main')],
                                        'question-mark'
                                    )
                                };
                            }
                        )
                    }
                ]
            };
        },
        _initializeCharts: function() {
            // Day summary.
            (new SummaryView({
                el: this.$('div[name=summary]'),
                model: this._forecast
            })).render();

            this._chart.setData(this.chartData());
        },
        render: function() {
        }
    }
    );
