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
                    this._phases.fetch({
                        success: (function() {
                            this.$('select[name=phase_id]').val(this.model.get('phase_id'));
                        }).bind(this)
                    });

                    (new CollectionView({
                        el: this.$('select[name=phase_id]'),
                        model: this._phases,
                        view: StaticView.extend({
                            tagName: 'option',
                            attributes: function() {
                                return { value: this.model.get('phase_id') };
                            },
                            template: '<%-phase_desc%>'
                        })
                    })).render();

                    this.on('finished', this.save.bind(this));
                },
                // Save form options to the model.
                save: function() {
                    console.log('set ', this.$('select[name=phase_id]').val());
                    this.model.set({
                        phase_id: this.$('select[name=phase_id]').val()
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
                            attributes: function() {
                                return { value: this.model.get('kb_family_id') };
                            },
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
                            attributes: function() {
                                return { value: this.model.get('kb_variety_id') };
                            },
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
                        'kb/family/' + this.$('select[name=family]').val() + '/variety'
                        );
                    this._varieties.fetch({
                        success: (function() {
                            this.$('select[name=variety]').val(this.model.get('kb_variety_id'));
                        }).bind(this)
                    });
                },
                save: function() {
                    this.model.set({
                        kb_variety_id: this.$('select[name=variety]').val()
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
                    this._sensors.fetch({
                        success: (function() {
                            this.$('select[name=sensor]').val(this.model.get('sensor_id'));
                        }).bind(this)
                    });
                    (new CollectionView({
                        el: this.$('select[name=sensor]'),
                        model: this._sensors,
                        view: StaticView.extend({
                            tagName: 'option',
                            attributes: function() {
                                return { value: this.model.get('sensor_id') };
                            },
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

var BatchInfoView = StaticView.extend(
    {
        initialize: function() {
            StaticView.prototype.initialize.apply(this, arguments);
            StaticView.prototype.render.apply(this);

            var history = new (RestCollection.extend({
                model: (Backbone.Model.extend({
                    defaults: {
                        phase_desc: '',
                        start: '',
                        finish: 'now'
                    }
                })),
                url: '/api/batch/' + this.model.get('batch_id') + '/history',
                comparator: function(model) {
                    return model.get('start');
                }
            }));
            history.fetch();

            (new CollectionView({
                el: this.$('ul[name=history]'),
                view: StaticView.extend({
                    tagName: 'li',
                    template: '<%-phase_desc%> (from <%-start%> to <%-finish%>)'
                }),
                model: history
            })).render();
        },
        render: function() {
        },
        template: '\
            <h1>Batch of <%-kb_family_cname%> <%-kb_variety_lname%> (<%-kb_variety_cname%>)</h1>\
            <h2>History</h2>\
            <ul name="history"></ul>\
            '
    }
    );

var ChangePhaseForm = StaticView.extend(
    {
        initialize: function() {
            StaticView.prototype.initialize.apply(this, arguments);
            StaticView.prototype.render.apply(this);
            var phases = new PhaseCollection;
            phases.fetch();
            (new CollectionView({
                el: this.$('select[name=phase]'),
                model: phases,
                view: StaticView.extend({
                    tagName: 'option',
                    attributes: function() {
                        return { value: this.model.get('phase_id') };
                    },
                    template: '<%-phase_desc%>'
                })
            })).render();
            this.on('ok', this.save.bind(this));
        },
        render: function() {
        },
        template: '\
        <h1>Change Phase</h1>\
        <label>\
            New Phase\
            <select name="phase"></select>\
        </label>\
        ',
        save: function() {
            this.model.set('phase_id', this.$('select[name=phase]').val());
        }
    }
    );

var PhaseView = StaticView.extend(
    {
        initialize: function() {
            StaticView.prototype.initialize.apply(this, arguments);
            StaticView.prototype.render.apply(this);

            var batches = new BatchCollection(
                [],
                {
                    url: (function() {
                        return restUri('phase/' + this.model.get('phase_id') + '/batch');
                    }).bind(this)
                }
                );
            this._batches = batches;
            this._batches.fetch();

            (new CollectionView({
                el: this.$('ul[name=batches]'),
                model: this._batches,
                view: StaticView.extend({
                    tagName: 'li',
                    template: '\
                    <div class="box">\
                    <div class="batch" style="border-color: <%-kb_variety_colour%>;">\
                    <%-kb_family_cname%> <%-kb_variety_lname%> (<%-kb_variety_cname%>)\
                    </div>\
                    </div>\
                    ',
                events: {
                    click: 'showBatchInfo'
                },
                showBatchInfo: function() {
                    var m = new Modal({
                        model: this.model,
                        view: BatchInfoView,
                        buttons: [
                            new ModalButton({ name: 'move', label: 'Plant On' }),
                            StandardButton.close()
                        ]
                    });
                    this.listenTo(m, 'move', this.moveOn.bind(this));
                    gApplication.modal(m);
                },
                moveOn: function() {
                    var m = new Modal({
                        model: this.model,
                        view: ChangePhaseForm,
                        buttons: [
                            StandardButton.cancel(),
                            StandardButton.ok()
                        ]
                    });
                    this.listenTo(
                            m,
                            'ok',
                            (function() {
                                this.model.save({
                                    success: function() {
                                        gApplication.currentPage().reset()
                                    }
                                });
                                console.log('moved on to', this.model.get('phase_id'));
                            }).bind(this)
                            );
                    gApplication.modal(m);
                }
                })
            })).render();
        },
        template: '<h2><%-phase_desc%></h2><ul class="batches" name="batches"></ul>',
        reload: function() {
            this._batches.fetch();
        },
        render: function() {
        }
    }
    );

var BoardPage = PageView.extend(
    {
        pageTitle: 'Progress Board',
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
        reset: function() {
            this._phases.fetch();
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

