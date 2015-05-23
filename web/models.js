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
            setOrder: function() {
                this.each(
                    function(model, i) {
                        model.set('phase_order', i);
                    }
                    );
            }
        }
        );

