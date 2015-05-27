var restUri = function(fragment) {
    return 'api/' + fragment;
};

var Phase = RestModel.extend(
        {
            defaults: {
                phase_desc: '',
                phase_order: 0
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
                kb_variety_id: 0
            },
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
                kb_variety_weeks: '',
                kb_family_id: 0
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

