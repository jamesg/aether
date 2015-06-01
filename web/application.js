var Colour = Backbone.Model.extend(
        {
            defaults: {
                code: '#000000',
                name: 'Black'
            }
        }
        );

var ColourCollection = Backbone.Collection.extend(
        {
            model: Colour
        }
        );

var PlantPalette = [
    { code: '#b90846', name: 'Purple' },
    { code: '#612c48', name: 'Purple' },
    { code: '#a40018', name: 'Purple' },
    { code: '#af5d85', name: 'Purple' },
    { code: '#652542', name: 'Purple' },
    { code: '#ecc7d5', name: 'Pink' },
    { code: '#9f9faa', name: 'Blue' },
    { code: '#4d5615', name: 'Green' },
    { code: '#bac467', name: 'Green' },
    { code: '#95a73b', name: 'Green' },
    { code: '#3e5124', name: 'Green' },
    { code: '#3b6a4d', name: 'Green' },
    { code: '#87a625', name: 'Green' },
    { code: '#dade63', name: 'Green' },
    { code: '#bac584', name: 'Green' },
    { code: '#ca6e1b', name: 'Orange' },
    { code: '#f8a81b', name: 'Orange' },
    { code: '#ed8a22', name: 'Orange' },
    { code: '#ab6529', name: 'Orange' },
    { code: '#f2cf3d', name: 'Orange' },
];

var ColourPicker = StaticView.extend(
        {
            initialize: function(options) {
                StaticView.prototype.initialize.apply(this, arguments);
                StaticView.prototype.render.apply(this);

                this._attribute = options['attribute'];
                var palette = coalesce(options['palette'], PlantPalette);

                (new CollectionView({
                    el: this.$('div[name=colour]'),
                    model: new ColourCollection(palette),
                    view: StaticView.extend({
                        tagName: 'span',
                        templateParams: function() {
                            return {
                                code: this.model.get('code'),
                                group: options['attribute']
                            };
                        },
                        template: '\
                            <input type="radio" id="<%-group%>-<%-code%>" name="<%-group%>" value="<%-code%>"></input>\
                            <label for="<%-group%>-<%-code%>" style="background-color: <%-code%>;">\
                            </label>\
                        '
                    })
                })).render();
            },
            render: function() {
            },
            save: function() {
                console.log(
                        'save colour value',
                        this.$('input:radio[name=colourname]:checked').val()
                        );
                this.model.set(
                        this._attribute,
                        this.$('input:radio[name=colourname]:checked').val()
                        );
            },
            template: '\
                <div name="colour" class="colourpicker"></div>\
                '
        }
        );

