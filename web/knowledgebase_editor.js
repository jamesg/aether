var FamiliesPage = PageView.extend(
    {
        pageTitle: 'Families',
        template: $('#familiespage-template').html(),
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this);

            this._families = new FamilyCollection;
            this._families.fetch();

            var families = this._families;
            (new CollectionView({
                el: this.$('ul[name=families]'),
                model: this._families,
                view: StaticView.extend({
                    tagName: 'li',
                    template: '\
                    <%-kb_family_lname%> (<%-kb_family_cname%>, <%-kb_family_desc%>)\
                    <button type="button" name="edit">Edit</button>\
                    <button type="button" name="goto">Go To</button>\
                    ',
                    events: {
                        'click button[name=edit]': 'editFamily',
                        'click button[name=goto]': 'gotoFamily'
                    },
                    editFamily: function() {
                        var m = new Modal({
                            model: this.model,
                            view: FamilyForm,
                            buttons: [
                                StandardButton.cancel(),
                                StandardButton.destroy(),
                                StandardButton.save()
                            ]
                        });
                        this.listenTo(m, 'finished', families.fetch.bind(families));
                        gApplication.modal(m);
                    },
                    gotoFamily: function () {
                        gApplication.pushPage(
                            new FamilyPage({ model: this.model })
                            );
                    }
                })
            })).render();
        },
        events: {
            'click button[name=new]': 'createFamily'
        },
        createFamily: function() {
            var m = new Modal({
                model: new Family,
                view: FamilyForm,
                buttons: [
                    StandardButton.cancel(),
                    StandardButton.create()
                ]
            });
            this.listenTo(m, 'finished', this._families.fetch.bind(this._families));
            gApplication.modal(m);
        },
        render: function() {
        }
    }
    );

var FamilyPage = PageView.extend(
    {
        pageTitle: function() { return this.model.get('kb_family_cname'); },
        template: $('#familypage-template').html(),
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this);

            this._varieties = new (VarietyCollection.extend({
                url: restUri(
                     'kb/family/' + this.model.id + '/variety'
                     )
            }));
            this._varieties.fetch();

            (new CollectionView({
                model: this._varieties,
                el: this.$('ul[name=varieties]'),
                view: StaticView.extend({
                    tagName: 'li',
                    template: '\
                    <%-kb_variety_lname%> (<%-kb_variety_cname%>)\
                    ',
                    events: {
                        click: 'gotoVariety'
                    },
                    gotoVariety: function () {
                        var m = new Modal({
                            model: this.model,
                            view: VarietyForm,
                            buttons: [
                                StandardButton.cancel(),
                                StandardButton.destroy(),
                                StandardButton.save()
                            ]
                        });
                        gApplication.modal(m);
                    }
                })
            })).render();
        },
        events: {
            'click button[name=new]': 'newVariety'
        },
        newVariety: function() {
            var v = new Variety;
            v.set('kb_family_id', this.model.get('kb_family_id'));
            var m = new Modal({
                model: v,
                view: VarietyForm,
                buttons: [
                    StandardButton.cancel(),
                    StandardButton.create()
                ]
            });
            this.listenTo(m, 'finished', this._varieties.fetch.bind(this._varieties));
            gApplication.modal(m);
        },
        render: function() {
        }
    }
    );

var FamilyForm = StaticView.extend(
    {
        template: $('#familyform-template').html(),
        initialize: function() {
            StaticView.prototype.initialize.apply(this, arguments);
            StaticView.prototype.render.apply(this);
            this.on('create', this.save.bind(this));
            this.on('destroy', this.destroy.bind(this));
            this.on('save', this.save.bind(this));
        },
        save: function() {
            this.model.set({
                kb_family_cname: this.$('input[name=kb_family_cname]').val(),
                kb_family_lname: this.$('input[name=kb_family_lname]').val(),
                kb_family_desc: this.$('input[name=kb_family_desc]').val()
            });
            this.model.save(
                {},
                { success: this.trigger.bind(this, 'finished') }
                );
        },
        destroy: function() {
            gApplication.modal(
                new ConfirmModal({
                    message: 'Delete this family (including all varieties)?',
                    callback: (function() {
                        this.model.destroy({
                            success: this.trigger.bind(this, 'finished')
                        });
                    }).bind(this)
                })
                );
        },
        render: function() {
        }
    }
    );

var VarietyForm = StaticView.extend(
    {
        template: $('#varietyform-template').html(),
        initialize: function() {
            StaticView.prototype.initialize.apply(this, arguments);
            StaticView.prototype.render.apply(this, arguments);
            this._colourPicker = new ColourPicker({
                el: this.$('div[name=colourpicker]'),
                attribute: 'kb_variety_colour',
                model: this.model,
                palette: PlantPalette
            });
            this._colourPicker.render();
            if(!this.model.isNew())
                this.model.fetch();
            this.on('create', this.save.bind(this));
            this.on('destroy', this.destroy.bind(this));
            this.on('save', this.save.bind(this));
        },
        render: function() {
        },
        save: function() {
            this.model.set({
                kb_variety_cname: this.$('input[name=kb_variety_cname]').val(),
                kb_variety_lname: this.$('input[name=kb_variety_lname]').val(),
                kb_edible_part: this.$('input[name=edible]').prop('checked') ?
                    this.$('select[name=ediblepart]').val() : null,
                aether_kb_variety_container:
                    this.$('input[name=aether_kb_variety_container]').prop('checked'),
                aether_kb_variety_flower:
                    this.$('input[name=aether_kb_variety_flower]').prop('checked'),
                aether_kb_variety_perennial:
                    this.$('input[name=aether_kb_variety_perennial]').prop('checked'),
                aether_kb_variety_prefer_shade:
                    this.$('input[name=aether_kb_variety_prefer_shade]').prop('checked'),
                aether_kb_variety_prefer_sun:
                    this.$('input[name=aether_kb_variety_prefer_sun]').prop('checked'),
                kb_variety_colour: this._colourPicker.colour(),
                harvest_mon: _.filter(
                    _.range(1, 13),
                    function(i) { return this.$('#harvest_' + i).prop('checked'); },
                    this
                    ),
                plant_mon: _.filter(
                    _.range(1, 13),
                    function(i) { return this.$('#plant_' + i).prop('checked'); },
                    this
                    ),
                sow_mon: _.filter(
                    _.range(1, 13),
                    function(i) { return this.$('#sow_' + i).prop('checked'); },
                    this
                    )
            });
            //this._colourPicker.save();
            console.log('attributes', this.model.attributes);
            this.model.save(
                {},
                { success: this.trigger.bind(this, 'finished') }
                );
        },
        destroy: function() {
            gApplication.modal(
                new ConfirmModal({
                    message: 'Delete this variety?',
                    callback: (function() {
                        this.model.destroy({
                            success: this.trigger.bind(this, 'finished')
                        });
                    }).bind(this)
                })
                );
        }
    }
    );
