var SensorsPage = PageView.extend(
    {
        pageTitle: 'Sensors',
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this);

            var phases = new PhaseCollection;
            phases.fetch();
            var batches = new BatchCollection;
            batches.fetch();
            var sensors = new SensorCollection;
            sensors.fetch();

            (new TableView({
                    el: this.$('table[name=phases]'),
                    model: phases,
                    theadView: StaticView.extend({
                        tagName: 'thead',
                        template: '<th>Name</th><th>Temperature</th>'
                    }),
                    trView: StaticView.extend({
                        tagName: 'tr',
                        template: '<td><%-phase_desc%></td><td><%-temperature%></td>',
                        events: {
                            click: 'gotoPhase'
                        },
                        gotoPhase: function() {
                            gApplication.pushPage(
                                new SensorForecastPage({ model: this.model })
                            )
                        }
                    })
            })).render();
            (new TableView({
                el: this.$('table[name=batches]'),
                model: batches,
                filter: function(batch) {
                    return (batch.get('sensor_id') != null);
                },
                theadView: StaticView.extend({
                    tagName: 'thead',
                    template: '<th>Variety</th>'
                }),
                trView: StaticView.extend({
                    tagName: 'tr',
                    template: '<td><%-kb_variety_cname%></td>'
                })
            })).render();
            (new TableView({
                el: this.$('table[name=sensors]'),
                model: sensors,
                theadView: StaticView.extend({
                    tagName: 'thead',
                    template: '<th>Description</th><th>Location</th><th>Moisture</th><th>Temperature</th>'
                }),
                trView: StaticView.extend({
                    tagName: 'tr',
                    template: '\
                        <td><%-sensor_desc%></td>\
                        <td><%-phase_desc%></td>\
                        <td><%if(aether_moisture_sensor){%><%}%></td>\
                        <td><%if(aether_temperature_sensor){%><%}%></td>\
                        ',
                    events: {
                        'click': 'gotoSensor'
                    },
                    gotoSensor: function() {
                        gApplication.pushPage(new SensorPage({ model: this.model }));
                    }
                }),
                emptyView: StaticView.extend({
                    template: 'no sensors'
                })
            })).render();
        },
        template: $('#sensorspage-template').html(),
        render: function() {
        }
    }
    );

var SensorPage = PageView.extend(
        {
            pageTitle: function() {
                return this.model.get('sensor_desc');
            },
            initialize: function() {
                PageView.prototype.initialize.apply(this, arguments);
                PageView.prototype.render.apply(this);
                this.model.fetch();
            }
        }
        );

var SensorForecastPage = PageView.extend(
    {
        pageTitle: function() {
            return this.model.get('phase_desc') + ' Sensors';
        },
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this);

            var dailyForecast = new DailyForecastCollection;
            dailyForecast.fetch();
            var page = this;
            this._daysView = new CollectionView({
                el: this.$('p[name=days]'),
                model: dailyForecast,
                filter: function(model) {
                    return model.get('detailed_available');
                },
                view: StaticView.extend({
                    tagName: 'button',
                    className: 'tag',
                    template: '<%-str%>',
                    templateParams: function() {
                        return {
                            str: moment(this.model.get('date')).format('dddd Do MMMM')
                        };
                    },
                    events: {
                        click: 'gotoDay'
                    },
                    gotoDay: function() {
                        page.gotoDay(this.model.get('date'));
                    }
                })
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

            this._forecast = new ForecastCollection;
            this.gotoDay(moment().format('YYYY-MM-DD'));
            this._chart = new Chart({ id: 'temperaturechart', type: 'line' })
        },
        template: $('#sensorforecastpage-template').html(),
        render: function() {
        },
        gotoDay: function(date) {
            this._forecast.fetch({
                // Provide the timezone offset in minutes so that the
                // server will only provide weather for one day at the
                // local timezone.
                url: restUri(
                    'weather/day/' + date +
                    '?timezone=' + (new Date).getTimezoneOffset() +
                    '&phase_id=' + this.model.get('phase_id')
                    ),
                success: (function() {
                    this.$('h1[name=date]').html(
                        'Temperature Model for ' +
                        moment(date).format('dddd Do MMMM YYYY')
                    );
                    console.log('chartdata', this.chartData());
                    this._chart.setData(this.chartData());
                }).bind(this)
            });

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
                    },
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
                    }
                ]
            };
        }
    }
)
