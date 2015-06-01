var SettingsPage = PageView.extend(
    {
        pageTitle: 'Settings',
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this);
            var phases = new PhaseCollection;
            phases.fetch();
            (new CollectionView({
                model: phases,
                el: this.$('p[name=current-phases]'),
                view: StaticView.extend({
                    tagName: 'span',
                    className: 'tag',
                    template: '<%-phase_desc%>'
                })
            })).render();
        },
        render: function() {
        },
        template: $('#settingspage-template').html(),
        events: {
            'click button[name=phases]': 'showPhases'
        },
        showPhases: function() {
            gApplication.pushPage(PhasesPage);
        }
    }
    );

var PhaseForm = StaticView.extend(
    {
        template: $('#phaseform-template').html(),
        save: function() {
            this.model.set({
                phase_desc: this.$('input[name=phase_desc]').val()
            });
            this.model.save(
                {},
                {
                    success: (function() {
                        this.trigger('finished');
                    }).bind(this)
                }
                )
        },
        initialize: function() {
            StaticView.prototype.initialize.apply(this, arguments);
            this.on('save', this.save.bind(this));
        }
    }
    );

var PhasesPage = PageView.extend(
    {
        pageTitle: 'Phases',
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this, arguments);

            this._phases = new PhaseCollection;
            this._phases.fetch();
            var phases = this._phases;

            this._phasesView = new CollectionView({
                el: this.$('ul[name=phases]'),
                view: StaticView.extend({
                    tagName: 'li',
                    template: '\
                    <span name="phase-desc"><%-phase_desc%></span>\
                    <span class="visible-on-hover">\
                    <button type="button" name="up">Move up</button>\
                    <button type="button" name="down">Move down</button>\
                    <button type="button" name="destroy">Delete</button>\
                    <button type="button" name="edit">Edit</button>\
                    </span>\
                    ',
                    events: {
                        'click button[name=up]': 'moveUp',
                        'click button[name=down]': 'moveDown',
                        'click button[name=hide]': 'hide',
                        'click button[name=edit]': 'edit',
                        'click button[name=destroy]': 'destroy',
                        'click span[name=phase-desc]': 'edit'
                    },
                    moveUp: function() {
                        phases.moveUp(this.model);
                        phases.save();
                    },
                    moveDown: function() {
                        phases.moveDown(this.model);
                        phases.save();
                    },
                    edit: function() {
                        var m = new Modal({
                            model: this.model,
                            view: PhaseForm,
                            buttons: [
                                StandardButton.cancel(),
                                StandardButton.save()
                            ]
                        });
                        gApplication.modal(m);
                    },
                    destroy: function() {
                        var m = new ConfirmModal({
                            callback: (function() {
                                this.model.destroy();
                            }).bind(this),
                            message: 'Are you sure you want to delete ' +
                                this.model.get('phase_desc') + '?'
                        });
                        gApplication.modal(m);
                    }
                }),
                model: this._phases
            });
            this._phasesView.render();
        },
        events: {
            'click button[name=new-phase]': 'create'
        },
        create: function() {
            var m = new Modal({
                model: new Phase,
                view: PhaseForm,
                buttons: [ StandardButton.cancel(), StandardButton.create() ]
            });
            gApplication.modal(m);
            this.listenTo(
                    m,
                    'finished',
                    function() {
                        this._phases.fetch(),
                        this._phases.save()
                    }
                    );
        },
        render: function() {},
        template: $('#phasespage-template').html(),
    }
    );

