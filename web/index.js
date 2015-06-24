var HomePage = PageView.extend(
    {
        pageTitle: 'Home',
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this);
            var sowVarieties = new VarietyCollection;
            sowVarieties.fetch({
                url: restUri('kb/month/' + moment().format('M') + '/sow/variety')
            });
            (new CollectionView({
                el: this.$('ul[name=sow-varieties]'),
                view: StaticView.extend({
                    tagName: 'li',
                    template: '<%-kb_variety_cname%>'
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
                    template: '<%-kb_variety_cname%>'
                }),
                model: plantVarieties
            })).render();
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

