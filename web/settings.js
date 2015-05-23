var SettingsPage = PageView.extend(
    {
        pageTitle: 'Settings',
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
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

var PhasesPage = PageView.extend(
    {
        pageTitle: 'Phases',
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this, arguments);

            this._phases = new PhaseCollection;
            this._phases.fetch();

            (new CollectionView({
                el: this.$('ul[name=phases]'),
                view: StaticView.extend({
                    tagName: 'li',
                    template: '\
                    <span name="phase-desc"><%-phase_desc%></span>\
                    <span class="visible-on-hover">\
                    <button type="button" name="up">Move up</button>\
                    <button type="button" name="down">Move down</button>\
                    <button type="button" name="hide">Delete</button>\
                    <button type="button" name="edit">Edit</button>\
                    </span>\
                    ',
                events: {
                    'click button[name=up]': 'moveUp',
                    'click button[name=down]': 'moveDown',
                    'click button[name=hide]': 'hide',
                    'click button[name=edit]': 'edit',
                    'click span[name=phase-desc]': 'edit'
                },
                edit: function() {
                    var m = new Modal({
                        modal: this.model,
                        view: StaticView.extend({
                            template: $('#phaseform-template').html()
                        }),
                        buttons: [ StandardButton.cancel(), StandardButton.save(function() { console.log('todo save phase'); }) ]
                    });
                    gApplication.modal(m);
                }
                }),
                model: this._phases
            })).render();
        },
        render: function() {},
        template: $('#phasespage-template').html(),
    }
    );

