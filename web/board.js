var formatUtcDate = function(str) {
    return moment.unix(str).local().format('dddd, Do MMMM YYYY h:mma');
};

var NewBatchWizard = function() {
    _.extend(this, Backbone.Events);
    this.model = new Batch;
};

NewBatchWizard.prototype = {
    showPhaseDialog: function() {
        var m = new Modal({
            model: this.model,
            help: StaticView.extend({
                template: '\
                <h2>Choosing a Start Location</h2>\
                <p>Choose a starting location for the plants.</p>\
                '
            }),
            view: StaticView.extend({
                initialize: function() {
                    StaticView.prototype.initialize.apply(this, arguments);
                    StaticView.prototype.render.apply(this);
                    this._phases = new PhaseCollection;
                    this._phases.fetch({
                        success: (function() {
                            if(this.model.has('phase_id'))
                                this.$('select[name=phase_id]').val(this.model.get('phase_id'));
                            else if(this._phases.length > 0)
                                this.$('select[name=phase_id]').val(this._phases.at(0).get('phase_id'));
                        }).bind(this)
                    });

                    (new CollectionView({
                        el: this.$('select[name=phase_id]'),
                        model: this._phases,
                        view: StaticView.extend({
                            tagName: 'option',
                            attributes: function() {
                                return {
                                    value: this.model.get('phase_id')
                                };
                            },
                            template: '<%-phase_desc%>'
                        })
                    })).render();

                    this.on('next', this.save.bind(this));
                },
                // Save form options to the model.
                save: function() {
                    this.model.set({
                        phase_id: this.$('select[name=phase_id]').val()
                    });
                },
                template: $('#phaseform-template').html(),
                render: function() {
                }
            }),
            buttons: [ StandardButton.cancel(), StandardButton.next() ]
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

                    this.on('prev', this.save.bind(this));
                    this.on('next', this.save.bind(this));
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
            help: StaticView.extend({
                template: '\
                <h2>Choosing a Variety</h2>\
                <p>Choose the variety you are about to sow or plant.</p>\
                '
            }),
            buttons: [
                StandardButton.cancel(),
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
                            this.$('select[name=sensor]').val(
                                this.model.has('sensor_id') ?
                                    this.model.get('sensor_id') :
                                    this._sensors.at(0).get('sensor_id')
                            );
                        }).bind(this)
                    });
                    (new CollectionView({
                        el: this.$('select[name=sensor]'),
                        model: this._sensors,
                        view: StaticView.extend({
                            tagName: 'option',
                            attributes: function() {
                                return { value: this.model.id };
                            },
                            template: '<%-sensor_desc%>'
                        })
                    })).render();

                    this.on('prev', this.save.bind(this));
                    this.on('next', this.save.bind(this));
                },
                save: function() {
                    var sensor = this._sensors.at(
                        this.$('select[name=sensor]')[0].selectedIndex
                        );
                    if(sensor) {
                        this.model.set({ sensor_id: sensor.id });
                        console.log('set sensor', sensor, sensor.id)
                    }
                },
                events: {
                    'change input[name=movesensor]': 'checkSensorEnabled'
                },
                checkSensorEnabled: function() {
                    console.log('check', this.$('input[name=movesensor]').prop('checked'))
                    this.$('select[name=sensor]').prop(
                        'disabled',
                        !this.$('input[name=movesensor]').prop('checked')
                    );
                },
                template: $('#sensorform-template').html(),
                render: function() {
                }
            }),
            help: StaticView.extend({
                template: '\
                <h2>Choosing a Sensor</h2>\
                <p>If you are going to move a sensor to this batch as soon \
                as it is planted, select the sensor here.  It will be \
                unassigned from any batches currently monitored.</p>\
                '
            }),
            buttons: [
                StandardButton.cancel(),
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

                    if(this.model.has('sensor_id')) {
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
                StandardButton.cancel(),
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
        reset: function() {
            this._history.fetch();
        },
        initialize: function() {
            StaticView.prototype.initialize.apply(this, arguments);
            StaticView.prototype.render.apply(this);
            this.render();

            this._history = new (RestCollection.extend({
                model: (Backbone.Model.extend({
                    defaults: {
                        phase_desc: '',
                        start: '',
                        finish: ''
                    }
                })),
                url: '/api/batch/' + this.model.get('batch_id') + '/history',
                comparator: function(model) {
                    return model.get('start');
                }
            }));
            this._history.fetch();

            (new CollectionView({
                el: this.$('ul[name=history]'),
                view: StaticView.extend({
                    tagName: 'li',
                    template: '<%-phase_desc%> (from <%-start%> to <%-finish%>)',
                    templateParams: function() {
                        var params = _.clone(this.model.attributes);
                        params.start = formatUtcDate(this.model.get('start'));
                        if(this.model.get('finish') == '')
                            params.finish = 'now';
                        else
                            params.finish = formatUtcDate(this.model.get('finish'));
                        return params;
                    }
                }),
                model: this._history
            })).render();
        },
        render: function() {
            if(this.model.get('sensor_id') == null) {
                (new StaticView({
                    el: this.$('div[name=sensor]'),
                    template: 'No sensor is assigned to this batch.'
                })).render();
            } else {
                var sensor =
                    new Sensor({ sensor_id: this.model.get('sensor_id') });
                sensor.fetch();
                (new StaticView({
                    el: this.$('div[name=sensor]'),
                    template: '<%-sensor_desc%>',
                    model: sensor
                })).render();
            }
        },
        template: '\
            <h1>Batch of <%-kb_family_cname%> <%-kb_variety_lname%> (<%-kb_variety_cname%>)</h1>\
            <h2>Sensor</h2>\
            <div name="sensor"></div>\
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
            phases.fetch({
                success: (function() {
                    if(phases.length == 0) return;
                    var currentPhaseIndex = phases.indexOf(
                        phases.get(this.model.get('phase_id'))
                        );
                    this.$('select[name=phase]').val(
                        phases.at(
                            (phases.length > currentPhaseIndex) ?
                            currentPhaseIndex + 1 : 0
                            ).get('phase_id')
                        );
                }).bind(this)
            });
            (new CollectionView({
                el: this.$('select[name=phase]'),
                model: phases,
                filter: (function(phase) {
                    return (this.model.get('phase_id') != phase.get('phase_id'));
                }).bind(this),
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
        <h1>Change Location</h1>\
        <label>\
            New Location\
            <select name="phase"></select>\
        </label>\
        ',
        save: function() {
            this.model.set('phase_id', this.$('select[name=phase]').val());
            this.model.save(
                {},
                { success: this.trigger.bind(this, 'finished') }
            );
        }
    }
    );

var MoveSensorForm = StaticView.extend(
    {
        template: $('#movesensorform-template').html(),
        reset: function() {
            this._sensors.fetch();
        },
        initialize: function() {
            StaticView.prototype.initialize.apply(this, arguments);
            StaticView.prototype.render.apply(this);
            this._sensors = new SensorCollection;
            this._sensors.fetch({
                success: (function() {
                    this.$('select[name=sensor]').val(
                        this.model.has('sensor_id') ?
                            this.model.get('sensor_id') :
                            this._sensors.at(0).get('sensor_id')
                    );
                }).bind(this)
            });
            (new CollectionView({
                el: this.$('select[name=sensor]'),
                model: this._sensors,
                view: StaticView.extend({
                    tagName: 'option',
                    attributes: function() {
                        return { value: this.model.id };
                    },
                    template: '<%-sensor_desc%>'
                })
            })).render();

            this.on(
                'save',
                (function() {
                    var sensor = this._sensors.at(
                        this.$('select[name=sensor]')[0].selectedIndex
                        );
                    if(sensor) {
                        this.model.set({ sensor_id: sensor.id });
                        this.model.save(
                            {},
                            {
                                success: (function() {
                                    this.trigger('finished');
                                }).bind(this)
                            }
                        );
                    }
                }).bind(this)
            );
        },
        render: function() {
        }
    }
);

var SplitBatchForm = StaticView.extend(
    {
        template: $('#splitbatchform-template').html(),
        reset: function() {
            this._phases.fetch();
        },
        initialize: function() {
            StaticView.prototype.initialize.apply(this, arguments);
            StaticView.prototype.render.apply(this);
            this._phases = new PhaseCollection;
            this._phases.fetch({
                success: (function() {
                    this.$('select[name=phase]').val(
                        this.model.has('phase_id') ?
                            this.model.get('phase_id') :
                            this._phases.at(0).get('phase_id')
                    );
                }).bind(this)
            });
            (new CollectionView({
                el: this.$('select[name=phase]'),
                model: this._phases,
                view: StaticView.extend({
                    tagName: 'option',
                    attributes: function() {
                        return { value: this.model.id };
                    },
                    template: '<%-phase_desc%>'
                })
            })).render();

            this.on(
                'save',
                (function() {
                    var phase = this._phases.at(
                        this.$('select[name=phase]')[0].selectedIndex
                    );
                    var newBatch = new Batch;
                    newBatch.fetch({
                        url: restUri('batch/' + this.model.id + '/copy'),
                        success: (function() {
                            newBatch.save(
                                {
                                    phase_id: phase.id
                                },
                                {
                                    success: (function() {
                                        this.trigger('finished');
                                    }).bind(this)
                                }
                            )
                        }).bind(this)
                    })
                }).bind(this)
            );
        },
        render: function() {
        }
    }
);

var PhaseView = StaticView.extend(
    {
        initialize: function(options) {
            StaticView.prototype.initialize.apply(this, arguments);
            StaticView.prototype.render.apply(this);

            var phase_id = options.phase_id;
            // var batches = new BatchCollection(
            //     [],
            //     {
            //         url: (function() {
            //             return restUri('phase/' + this.model.get('phase_id') + '/batch');
            //         }).bind(this)
            //     }
            //     );
            // this._batches = batches;
            // this._batches.fetch();

            (new CollectionView({
                el: this.$('ul[name=batches]'),
                model: options.batches,
                filter: function(batch) {
                    return (batch.get('phase_id') == phase_id);
                },
                emptyView: StaticView.extend({
                    template: '\
                    <div class="box">There are no batches in this phase.</div>\
                    '
                }),
                view: StaticView.extend({
                    tagName: 'li',
                    template: '\
                    <div class="box">\
                    <div class="batch" style="border-color: <%-kb_variety_colour%>;">\
                    <table>\
                    <tr>\
                        <td>Name</td>\
                        <td><%-kb_family_cname%> <%-kb_variety_lname%> (<%-kb_variety_cname%>)</td>\
                    </tr>\
                    <tr><td>Sown</td><td><%-sowing_s%></td></tr>\
                    <tr><td>Planted</td><td><%-planting_s%></td></tr>\
                    <%if(sensor_desc!=""){%>\
                        <tr>\
                        <td>Sensor</td>\
                        <td>\
                        <span class="oi" data-glyph="wifi" aria-hidden="true"> </span>\
                        <%-sensor_desc%></td>\
                        </tr>\
                    <%}%>\
                    </table>\
                    </div>\
                    </div>\
                    ',
                    templateParams: function() {
                        return _.extend(
                            _.clone(this.model.attributes),
                            {
                                sowing_s: formatUtcDate(this.model.get('sowing')),
                                planting_s: formatUtcDate(this.model.get('planting'))
                            }
                            );
                    },
                    events: {
                        click: 'showBatchInfo'
                    },
                    showBatchInfo: function() {
                        var infoModal = new Modal({
                            model: this.model,
                            view: BatchInfoView,
                            buttons: [
                                new ModalButton(
                                    { name: 'movesensor', label: 'Move Sensor' }
                                ),
                                new ModalButton(
                                    { name: 'move', label: 'Plant On' }
                                ),
                                new ModalButton(
                                    { name: 'split', label: 'Split' }
                                ),
                                StandardButton.close()
                            ],
                            help: StaticView.extend({
                                template: '\
                                <h2>Batch Information</h2>\
                                <p><emph>Move Sensor</emph> - Move a sensor to \
                                this batch of plants.</p>\
                                <p><emph>Plant On</emph> - Move the entire \
                                batch to a new location as plants grow.</p>\
                                <p><emph>Split</emph> - Move some of the \
                                plants to a new location, leaving some in the \
                                current location.</p>\
                                '
                            })
                        });
                        this.listenTo(
                            infoModal,
                            'move',
                            (function moveOn() {
                                var moveModal = new Modal({
                                    model: this.model,
                                    view: ChangePhaseForm,
                                    buttons: [
                                        StandardButton.cancel(),
                                        StandardButton.ok()
                                    ]
                                });
                                this.listenTo(
                                    moveModal,
                                    'finished',
                                    (function() {
                                        // Reload the history list.
                                        infoModal.view.reset();
                                        gApplication.currentPage().reset();
                                    }).bind(this)
                                );
                                this.listenTo(
                                    moveModal,
                                    'cancel',
                                    moveModal.finish.bind(moveModal)
                                );
                                gApplication.modal(moveModal);
                            }).bind(this)
                        );
                        this.listenTo(
                            infoModal,
                            'movesensor',
                            (function() {
                                var m = new Modal({
                                    view: MoveSensorForm,
                                    buttons: [ StandardButton.cancel(), StandardButton.save() ],
                                    model: this.model
                                });
                                gApplication.modal(m);
                            }).bind(this)
                        );
                        this.listenTo(
                            infoModal,
                            'split',
                            (function() {
                                var m = new Modal({
                                    view: SplitBatchForm,
                                    buttons: [ StandardButton.cancel(), StandardButton.save() ],
                                    model: this.model
                                });
                                gApplication.modal(m);
                            }).bind(this)
                        );
                        this.listenTo(infoModal, 'close', infoModal.finish.bind(infoModal));
                        gApplication.modal(infoModal);
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
            this.reset();
        },
        reset: function() {
            console.log('reset progress board');
            PageView.prototype.render.apply(this);

            this._phases = new PhaseCollection;
            this._phases.fetch();

            var batches = new BatchCollection;
            batches.fetch();

            this._phasesView = new CollectionView({
                el: this.$('div[name=phases]'),
                model: this._phases,
                view: PhaseView,
                constructView: function(model) {
                    return new PhaseView({
                        phase_id: model.get('phase_id'),
                        model: model,
                        batches: batches
                    });
                },
                emptyView: StaticView.extend({
                    tagName: 'p',
                    className: 'advice',
                    template: '\
                    There are currently no phases.  Go to the \
                    <a href="settings.html">settings page</a> to set up a \
                    phase before creating a batch.\
                    '
                })
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
