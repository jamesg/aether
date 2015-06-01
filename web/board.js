var NewBatchWizard = function() {
    _.extend(this, Backbone.Events);
    this.model = new Batch;
};

NewBatchWizard.prototype = {
    showPhaseDialog: function() {
        var m = new Modal({
            model: this.model,
            view: StaticView.extend({
                initialize: function() {
                    StaticView.prototype.initialize.apply(this, arguments);
                    StaticView.prototype.render.apply(this);

                    this._phases = new PhaseCollection;
                    this._phases.fetch();

                    (new CollectionView({
                        el: this.$('select[name=phase_id]'),
                        model: this._phases,
                        view: StaticView.extend({
                            tagName: 'option',
                            template: '<%-phase_desc%>'
                        })
                    })).render();

                    this.on('finished', this.save.bind(this));
                },
                // Save form options to the model.
                save: function() {
                    this.model.set({
                        phase_id: this._phases.at(
                            this.$('select[name=phase_id]')[0].selectedIndex
                            ).get('phase_id')
                    });
                },
                template: $('#phaseform-template').html(),
                render: function() {
                }
            }),
            buttons: [ StandardButton.next() ]
        });
        this.listenTo(
            m,
            'next',
            (function() {
                m.finish();
                this.showVarietyDialog();
            }).bind(this)
            );
        gApplication.modal(m);
    },
    showVarietyDialog: function() {
        var m = new Modal({
            model: this.model,
            view: StaticView.extend({
                initialize: function() {
                    StaticView.prototype.initialize.apply(this, arguments);
                    StaticView.prototype.render.apply(this);

                    this._families = new FamilyCollection;

                    this._varieties = new VarietyCollection;

                    var familiesView = new CollectionView({
                        el: this.$('select[name=family]'),
                        model: this._families,
                        view: StaticView.extend({
                            tagName: 'option',
                            template: '<%-kb_family_cname%>'
                        })
                    });
                    this.listenTo(
                        this._families,
                        'reset',
                        this.updateVarieties.bind(this)
                        );
                    familiesView.render();

                    this._families.fetch({ reset: true });

                    (new CollectionView({
                        el: this.$('select[name=variety]'),
                        model: this._varieties,
                        view: StaticView.extend({
                            tagName: 'option',
                            template: '<%-kb_variety_cname%>'
                        })
                    })).render();

                    this.on('finished', this.save.bind(this));
                },
                events: {
                    'change select[name=family]': 'updateVarieties'
                },
                updateVarieties: function() {
                    this._varieties.url = restUri(
                        'kb/family/' +
                        this._families.at(
                            this.$('select[name=family]')[0].selectedIndex
                            ).get('kb_family_id') + '/variety'
                        );
                    console.log('fetch varieties');
                    this._varieties.fetch();
                },
                save: function() {
                    this.model.set({
                        kb_variety_id: this._varieties.at(
                            this.$('select[name=variety]')[0].selectedIndex
                            ).get('kb_variety_id')
                    });
                },
                template: $('#varietyform-template').html(),
                render: function() {
                }
            }),
            buttons: [
                StandardButton.prev(),
                StandardButton.next()
            ]
        });
        this.listenTo(
            m,
            'prev',
            (function() {
                m.finish();
                this.showPhaseDialog();
            }).bind(this)
            );
        this.listenTo(
            m,
            'next',
            (function() {
                m.finish();
                this.showSensorDialog();
            }).bind(this)
            );
        gApplication.modal(m);
    },
    showSensorDialog: function() {
        var m = new Modal({
            model: this.model,
            view: StaticView.extend({
                initialize: function() {
                    StaticView.prototype.initialize.apply(this, arguments);
                    StaticView.prototype.render.apply(this);

                    this._sensors = new SensorCollection;
                    this._sensors.fetch();
                    (new CollectionView({
                        el: this.$('select[name=sensor]'),
                        model: this._sensors,
                        view: StaticView.extend({
                            tagName: 'option',
                            template: '<%-sensor_desc%>'
                        })
                    })).render();

                    this.on('finished', this.save.bind(this));
                },
                save: function() {
                    var sensor = this._sensors.at(
                        this.$('select[name=sensor]')[0].selectedIndex
                        );
                    if(sensor)
                        this.model.set({ sensor_id: sensor.get('sensor_id') });
                },
                template: $('#sensorform-template').html(),
                render: function() {
                }
            }),
            buttons: [
                StandardButton.prev(),
                StandardButton.next()
            ]
        });
        this.listenTo(
            m,
            'prev',
            (function() {
                m.finish();
                this.showVarietyDialog();
            }).bind(this)
            );
        this.listenTo(
            m,
            'next',
            (function() {
                m.finish();
                this.showConfirmationDialog();
            }).bind(this)
            );
        gApplication.modal(m);
    },
    showConfirmationDialog: function() {
        var m = new Modal({
            model: this.model,
            view: StaticView.extend({
                initialize: function() {
                    StaticView.prototype.initialize.apply(this, arguments);
                    StaticView.prototype.render.apply(this);

                    var phase = new Phase;
                    phase.set({
                        phase_id: this.model.get('phase_id')
                    });
                    phase.fetch();
                    (new StaticView({
                        model: phase,
                        el: this.$('div[name=phase]'),
                        template: 'phase: <%-phase_desc%>'
                    })).render();
                    var variety = new Variety;
                    variety.set({
                        kb_variety_id: this.model.get('kb_variety_id')
                    });
                    variety.fetch();
                    (new StaticView({
                        model: variety,
                        el: this.$('div[name=variety]'),
                        template: 'variety: <%-kb_variety_cname%>'
                    })).render();

                    if(this.model.get('sensor_id') > 0) {
                        var sensor = new Sensor;
                        sensor.set({
                            sensor_id: this.model.get('sensor_id')
                        });
                        sensor.fetch();
                        (new StaticView({
                            model: sensor,
                            el: this.$('div[name=sensor]'),
                            template: 'sensor: <%-sensor_desc%>'
                        })).render();
                    } else {
                        (new StaticView({
                            el: this.$('div[name=sensor]'),
                            template: 'sensor: none'
                        })).render();
                    }
                },
                template: $('#confirmationform-template').html(),
                render: function() {
                }
            }),
            buttons: [
                StandardButton.prev(),
                StandardButton.create()
            ]
        });
        this.listenTo(
            m,
            'prev',
            (function() {
                m.finish();
                this.showSensorDialog();
            }).bind(this)
            );
        this.listenTo(
            m,
            'create',
            (function() {
                m.finish();
                console.log('saving new batch', this.model);
                this.model.save(
                    {},
                    {
                        success: (function() {
                            // Wait for the model to be created on the server
                            this.trigger('create');
                        }).bind(this)
                    }
                    );
            }).bind(this)
            );
        gApplication.modal(m);
    }
};

var PhaseView = StaticView.extend(
    {
        initialize: function() {
            StaticView.prototype.initialize.apply(this, arguments);
            StaticView.prototype.render.apply(this);

            this._batches = new BatchCollection(
                [],
                {
                    url: (function() {
                        return restUri('phase/' + this.model.get('phase_id') + '/batch');
                    }).bind(this)
                }
                );
            this._batches.fetch();

            (new CollectionView({
                el: this.$('ul[name=batches]'),
                model: this._batches,
                view: StaticView.extend({
                    className: 'batch',
                    template: '<%-kb_family_lname%> <%-kb_variety_lname%> (<%-kb_variety_cname%>)'
                })
            })).render();
        },
        template: '<h2><%-phase_desc%></h2><ul name="batches"></ul>',
        reload: function() {
            this._batches.fetch();
        },
        render: function() {
        }
    }
    );

var BoardPage = PageView.extend(
    {
        pageName: 'Progress Board',
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this);

            this._phases = new PhaseCollection;
            this._phases.fetch();

            this._phasesView = new CollectionView({
                el: this.$('div[name=phases]'),
                model: this._phases,
                view: PhaseView
            });
            this._phasesView.render();
        },
        template: this.$('#boardpage-template').html(),
        render: function() {
        },
        events: {
            'click button[name=new-batch]': 'newBatch'
        },
        newBatch: function() {
            var wizard = new NewBatchWizard;
            this.listenTo(
                wizard,
                'create',
                (function() {
                    this._phasesView.each(
                        function(view) {
                            view.reload();
                        }
                        );
                }).bind(this)
                );
            wizard.showPhaseDialog();
        }
    }
    );

