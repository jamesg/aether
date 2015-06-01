var HomePage = PageView.extend(
    {
        pageTitle: 'Home',
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this);
            this._colourPicker = new ColourPicker({
                model: new Backbone.Model,
                attribute: 'colour',
                el: this.$('#colourpicker')
            });
            this._colourPicker.render();
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

