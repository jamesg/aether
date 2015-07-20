var varietyListAttributes = function() {
    return {
        style: 'border-left-color: ' +
            this.model.get('kb_variety_colour') + ';'
    };
};

var WeatherCardView = StaticView.extend({
    tagName: 'div',
    template: '\
    <h2><a href="weather.html">Weather</a></h2>\
    <div class="weather-container">\
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
    &nbsp;\
    </div>\
    ',
    templateParams: function() {
        var params = _.clone(this.model.attributes);
        _.extend(
            params,
            {
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
    }
});

var HomePage = PageView.extend(
    {
        pageTitle: 'Home',
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this);

            var batches = new BatchCollection;
            batches.fetch({ url: restUri('/batch/recently_moved') });
            (new CollectionView({
                el: this.$('ul[name=batches]'),
                model: batches,
                limit: 3,
                view: StaticView.extend({
                    tagName: 'li',
                    attributes: varietyListAttributes,
                    templateParams: function() {
                        var params = _.clone(this.model.attributes);
                        _.extend(
                            params,
                            { date: moment.unix(params['start']).format('Do MMMM') }
                        );
                        return params;
                    },
                    template: '<%-kb_family_cname%> <%-kb_variety_cname%> \
                        (<%-phase_desc%>), moved <%-date%>'
                }),
                emptyView: StaticView.extend({
                    tagName: 'li',
                    template: 'There are no batches.'
                })
            })).render();

            var sowVarieties = new VarietyCollection;
            sowVarieties.fetch({
                url: restUri('kb/month/' + moment().format('M') + '/sow/variety')
            });
            (new CollectionView({
                el: this.$('ul[name=sow-varieties]'),
                view: StaticView.extend({
                    tagName: 'li',
                    attributes: varietyListAttributes,
                    template: '<%-kb_variety_cname%>'
                }),
                emptyView: StaticView.extend({
                    tagName: 'li',
                    template: 'There are no varieties suitable for sowing this month.'
                }),
                model: sowVarieties
            })).render();

            var plantVarieties = new VarietyCollection;
            plantVarieties.fetch({
                url: restUri('kb/month/' + moment().format('M') + '/plant/variety')
            });
            (new CollectionView({
                el: this.$('ul[name=plant-varieties]'),
                view: StaticView.extend({
                    tagName: 'li',
                    attributes: varietyListAttributes,
                    template: '<%-kb_variety_cname%>'
                }),
                emptyView: StaticView.extend({
                    tagName: 'li',
                    template: 'There are no varieties suitable for sowing this month.'
                }),
                model: plantVarieties
            })).render();

            var phases = new PhaseCollection;
            phases.fetch();
            (new TableView({
                el: this.$('table[name=phases]'),
                model: phases,
                theadView: StaticView.extend({
                    tagName: 'thead',
                    template: '<th>Location</th><th>Temperature</th>'
                }),
                trView: StaticView.extend({
                    tagName: 'tr',
                    template: '<td><%-phase_desc%></td><td><%-temperature%></td>'
                })
            })).render();

            var todaysWeather = new DailyForecast;
            todaysWeather.fetch({ url: restUri('/weather/today') });
            (new WeatherCardView({ model: todaysWeather, el: this.$('div[name=weather]')})).render();
        },
        render: function() {
        },
        template: $('#homepage-template').html(),
        events: {
            'click button[name=pick-colour]': 'pickColour'
        },
        pickColour: function() {
            this._colourPicker.save();
        }
    }
    );
