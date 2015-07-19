var restUri = function(fragment) {
    return 'api/' + fragment;
};

var Phase = RestModel.extend(
    {
        defaults: {
            phase_desc: '',
            phase_order: 0,
            temperature: null
        },
        idAttribute: 'phase_id',
        url: function() {
            return this.isNew() ?
                restUri('phase') :
                restUri('phase/'+this.get('phase_id'));
        }
    }
    );

var PhaseCollection = RestCollection.extend(
    {
        model: Phase,
        url: restUri('phase'),
        save: function(options) {
            console.log('save phases');
            this.setOrder();
            if(_.isUndefined(options))
                options = {};
            return this.sync('update', this, options);
        },
        setOrder: function() {
            this.each(
                function(model, i) {
                    model.set('phase_order', i);
                }
                );
        },
        // Swap elements at index i and index i + 1.
        swap: function(i) {
            var model = this.at(i);
            this.remove(model);
            this.add(model, { at: i + 1 });
        },
        moveUp: function(model) {
            var i = this.indexOf(model);
            if(i >= 1 && i < this.length)
                this.swap(i - 1);
        },
        moveDown: function(model) {
            var i = this.indexOf(model);
            if(i >= 0 && i < this.length - 1)
                this.swap(i);
        }
    }
    );

var Batch = RestModel.extend(
    {
        defaults: {
            kb_variety_id: 0,
            kb_family_cname: '',
            kb_family_lname: '',
            kb_variety_cname: '',
            kb_variety_lname: ''
        },
        idAttribute: 'batch_id',
        url: function() {
            return this.isNew() ?
                restUri('batch') :
                restUri('batch/' + this.get('batch_id'));
        }
    }
    );

var BatchCollection = RestCollection.extend(
    {
        model: Batch,
        url: restUri('batch')
    }
    );

//
// Knowledge base types.
//

var Family = RestModel.extend(
    {
        idAttribute: 'kb_family_id',
        defaults: {
            kb_family_cname: '',
            kb_family_lname: '',
            kb_family_desc: ''
        },
        url: function() {
            return restUri(
                this.isNew() ? 'kb/family' : 'kb/family/' + this.get('kb_family_id')
                );
        }
    }
    );

var FamilyCollection = RestCollection.extend(
    {
        model: Family,
        url: restUri('kb/family')
    }
    );

var Variety = RestModel.extend(
    {
        idAttribute: 'kb_variety_id',
        defaults: {
            kb_variety_cname: '',
            kb_variety_lname: '',
            kb_variety_weeks: 0,
            kb_family_id: 0,
            aether_kb_variety_container: false,
            aether_kb_variety_flower: false,
            aether_kb_variety_prefer_sun: false,
            aether_kb_variety_prefer_shade: false,
            harvest_mon: [],
            plant_mon: [],
            sow_mon: []
        },
        url: function() {
            return restUri(
                this.isNew() ? 'kb/variety' : 'kb/variety/' + this.get('kb_variety_id')
                );
        }
    }
    );

var VarietyCollection = RestCollection.extend(
    {
        model: Variety,
        url: restUri('kb/variety')
    }
    );

//
// Sensors.
//

var Sensor = RestModel.extend(
    {
        idAttribute: 'sensor_id',
        defaults: {
            sensor_desc: '',
            phase_desc: '',
            aether_temperature_sensor: false,
            aether_moisture_sensor: false
        },
        url: function() {
            return restUri(
                this.isNew() ? 'sensor' : 'sensor/' + this.get('sensor_id')
                );
        }
    }
    );

var SensorCollection = RestCollection.extend(
    {
        model: Sensor,
        url: restUri('sensor')
    }
    );

//
// Location singleton.
//

var Location = RestModel.extend(
    {
        defaults: {
            location_city: '',
            location_lat: '',
            location_lon: ''
        },
        url: restUri('location')
    }
    );

//
// Settings.
//

var Settings = RestModel.extend(
    {
        defaults: {
            permission_create_batch: false,
            permission_move_batch: false
        },
        url: restUri('settings')
    }
    );

//
// Weather Forecast.
//

// Short range forecast.  Available every three hours for roughly the next week.
var Forecast = RestModel.extend(
        {
            idAttribute: 'forecast_dt',
            defaults: {
                forecast_main_temp: 0.0,
                forecast_rain: 0
            },
            url: function() {
                restUri('weather/' + this.id);
            }
        }
        );

var ForecastCollection = RestCollection.extend(
        {
            model: Forecast
        }
        );

// Daily forecast.  Available for the next 5-16 days, depending on the location.
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

//
// Authentication.
//

var Session = RestModel.extend(
        {
            idAttribute: 'token',
            url: restUri('auth/session')
        }
        );

var User = RestModel.extend(
        {
            idAttribute: 'user_id',
            defaults: {
                username: '',
                password: '',
                atlas_user_enabled: false,
                atlas_user_super: false
            },
            url: function() {
                return restUri(this.isNew() ? 'auth/user' : 'auth/user/' + this.id);
            }
        }
        );

var UserCollection = RestCollection.extend(
        {
            model: User,
            url: restUri('auth/user')
        }
        );

var CurrentUser = User.extend(
        {
            url: function() {
                return restUri('auth/session/' + storage.get('token') + '/user');
            }
        }
        );
