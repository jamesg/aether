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

