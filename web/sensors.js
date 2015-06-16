var SensorsPage = PageView.extend(
    {
        pageTitle: 'Sensors',
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this);
            var sensors = new SensorCollection;
            sensors.fetch();
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
        events: {
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

