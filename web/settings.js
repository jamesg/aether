var SettingsPage = PageView.extend(
    {
        pageTitle: 'Settings',
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this);
            this._phases = new PhaseCollection;
            this._phases.fetch();
            (new CollectionView({
                model: this._phases,
                el: this.$('p[name=current-phases]'),
                view: StaticView.extend({
                    tagName: 'span',
                    className: 'tag',
                    template: '<%-phase_desc%>'
                })
            })).render();
            this._location = new Location;
            this._location.fetch();
            (new StaticView({
                el: this.$('p[name=current-location]'),
                model: this._location,
                template: '<%-location_city%> (<%-location_lat%>&deg; N, <%-location_lon%>&deg; E)'
            })).render();
        },
        render: function() {
        },
        reset: function() {
            this._phases.fetch();
            this._location.fetch();
        },
        template: $('#settingspage-template').html(),
        events: {
            'click button[name=phases]': 'showPhases',
            'click button[name=location]': 'showLocation',
            'click button[name=permissions]': 'showPermissions',
            'click button[name=users]': 'showUsers'
        },
        showPhases: function() {
            gApplication.pushPage(PhasesPage);
        },
        showLocation: function() {
            gApplication.pushPage(new LocationPage({ model: new Location }));
        },
        showPermissions: function() {
            gApplication.pushPage(new PermissionsPage({ model: new Settings }));
        },
        showUsers: function() {
            gApplication.pushPage(UsersPage);
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

var LocationPage = PageView.extend(
    {
        pageTitle: 'Location',
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            //this.model = new Location;
            this.model.fetch();
        },
        events: {
            'click button[name=update]': 'updateLocation'
        },
        updateLocation: function() {
            var m = new Modal({
                view: StaticView.extend({
                    template: $('#locationform-template').html(),
                    initialize: function() {
                        StaticView.prototype.initialize.apply(this, arguments);
                        this.model.fetch();
                        this.on(
                            'search',
                            (function() {
                                this.model.set(
                                    'location_city',
                                    this.$('input[name=location_city]').val()
                                    );
                                this.model.save(
                                    {},
                                    { url: restUri('location/search') }
                                    );
                            }).bind(this)
                            );
                        this.on(
                            'save',
                            (function() {
                                this.model.set({
                                    location_city: this.$('input[name=location_city]').val(),
                                    location_lat: this.$('input[name=location_lat]').val(),
                                    location_lon: this.$('input[name=location_lon]').val()
                                });
                                this.model.save(
                                    {},
                                    { success: this.trigger.bind(this, 'finished') }
                                    );
                            }).bind(this)
                            );
                    }
                }),
                model: new Location,
                buttons: [
                    StandardButton.cancel(),
                    new ModalButton({
                        label: 'Search',
                        name: 'search',
                        icon: 'magnifying-glass'
                    }),
                    StandardButton.save()
                ]
            });
            this.listenTo(
                    m,
                    'finished',
                    this.model.fetch.bind(this.model)
                    );
            gApplication.modal(m);
        },
        template: $('#locationpage-template').html()
    }
    );

var PermissionsPage = PageView.extend(
    {
        pageTitle: 'Permissions',
        template: $('#permissionspage-template').html(),
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this);
            this.model.fetch();
            this._messageBox = new MessageBox({
                el: this.$('div[name=messagebox]')
            });
            this._messageBox.render();
            (new StaticView({
                el: this.$('form'),
                template: $('#permissionsform-template').html(),
                model: this.model
            })).render();
        },
        render: function() {},
        events: {
            'submit form[name=permissions]': 'save',
            'click button[name=cancel]': 'cancel'
        },
        save: function() {
            this.model.set({
                permission_create_batch: this.$('input[name=permission_create_batch]').is(':checked'),
                permission_move_batch: this.$('input[name=permission_move_batch]').is(':checked')
            });
            this.model.save(
                {},
                {
                    success: (function() {
                        gApplication.popPage();
                    }).bind(this),
                    error: (function() {
                        this._messageBox.displayError('Saving permissions');
                    }).bind(this)
                }
                );
            return false;
        },
        cancel: function() {
            gApplication.popPage();
        }
    }
    );

var UserForm = StaticView.extend(
    {
        template: $('#userform-template').html(),
        initialize: function() {
            StaticView.prototype.initialize.apply(this, arguments);
            this.on('save', this.save.bind(this));
            this.on('create', this.save.bind(this));
        },
        save: function() {
            this.model.set({
                username: this.$('input[name=username]').val(),
                atlas_user_enabled: this.$('input[name=enabled]').is(':checked'),
                atlas_user_super: this.$('input[name=super]').is(':checked'),
            });
            this.model.save(
                {},
                {
                    success: this.trigger.bind(this, 'finished')
                }
                );
        }
    }
    );

var UsersPage = PageView.extend(
    {
        pageTitle: 'Users',
        template: $('#userspage-template').html(),
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this);
            var users = new UserCollection;
            this._users = users;
            this._users.fetch();
            (new TableView({
                el: this.$('table[name=users]'),
                theadView: StaticView.extend({
                    tagName: 'thead',
                    template: '<tr><th>Name</th><th>Account enabled</th><th>Administrator</th></tr>'
                }),
                trView: StaticView.extend({
                    tagName: 'tr',
                    template: '\
                        <td><%-username%></td>\
                        <td><%if(atlas_user_enabled){%>\
                            <span class="oi" data-glyph="check" aria-hidden="true"> </span>\
                        <%}else{%>\
                            <span class="oi" data-glyph="x" aria-hidden="true"> </span>\
                        <%}%></td>\
                        <td><%if(atlas_user_super){%>\
                            <span class="oi" data-glyph="check" aria-hidden="true"> </span>\
                        <%}else{%>\
                            <span class="oi" data-glyph="x" aria-hidden="true"> </span>\
                        <%}%></td>\
                        ',
                    events: {
                        click: 'edit'
                    },
                    edit: function() {
                        var m = new Modal({
                            view: UserForm,
                            model: this.model,
                            buttons: [
                                StandardButton.save(),
                                StandardButton.destroy(),
                                StandardButton.cancel()
                            ]
                        });
                        this.listenTo(m, 'finished', users.fetch.bind(users));
                        gApplication.modal(m);
                    }
                }),
                model: this._users
            })).render();
        },
        reset: function() {
            this._users.fetch();
        },
        events: {
            'click button[name=create]': 'create'
        },
        create: function() {
            var m = new Modal({
                view: UserForm,
                model: this.model,
                buttons: [
                    StandardButton.create(),
                    StandardButton.cancel()
                ]
            });
            this.listenTo(m, 'finished', this._users.fetch.bind(this._users));
            gApplication.modal(m);
        },
        render: function() {
        }
    }
    );

