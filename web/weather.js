var kelvinToCelsius = function(degK) {
    return degK - 273.15;
};

var WeatherPage = PageView.extend(
    {
        pageTitle: 'Weather',
        template: $('#weatherpage-template').html(),
        gotoDay: function(day) {
            this.$('h1[name=weather-title]').html(
                'Weather for ' +
                moment().add(day, 'days').format('dddd Do MMMM YYYY')
                );
            this._forecast.fetch({
                // Provide the timezone offset in minutes so that the
                // server will only provide weather for one day at the
                // local timezone.
                url: restUri(
                    'weather/day/' + day + '?timezone=' +
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
            this.gotoDay(0);

            var page = this;
            (new CollectionView({
                el: this.$('ul[name=days]'),
                view: StaticView.extend({
                    tagName: 'li',
                    template: '<%-date%>',
                    events: {
                        click: 'selectDay'
                    },
                    selectDay: function() {
                        console.log('select day', this.model.get('day'));
                        page.gotoDay(this.model.get('day'));
                    }
                }),
                model: new (Backbone.Collection.extend({
                    model: Backbone.Model.extend({
                        defaults: {
                            day: 0,
                            date: 'invalid'
                        }
                    })
                }))(
                    _.map(
                        _.range(0, 3),
                        function(day) {
                            return {
                                day: day,
                                date: moment().add(day, 'days').format('dddd Do')
                            };
                        }
                    )
                )
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
