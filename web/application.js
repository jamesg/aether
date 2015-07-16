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
                var selected = this.model.get(this._attribute);

                (new CollectionView({
                    el: this.$('div[name=colour]'),
                    model: new ColourCollection(palette),
                    view: StaticView.extend({
                        tagName: 'span',
                        templateParams: function() {
                            return {
                                code: this.model.get('code'),
                                group: options['attribute'],
                                selected: (selected == this.model.get('code'))
                            };
                        },
                        template: '\
                            <input type="radio" id="<%-group%>-<%-code%>" name="<%-group%>" value="<%-code%>" <%if(selected){%>checked<%}%>></input>\
                            <label for="<%-group%>-<%-code%>" style="background-color: <%-code%>;">\
                            </label>\
                        '
                    })
                })).render();
            },
            render: function() {
            },
            // Get the currently selected colour value, or 'undefined' if no colour is selected.
            colour: function() {
                return this.$('input:radio[name=' + this._attribute + ']:checked').val();
            },
            save: function() {
                console.log(
                        'save colour value',
                        this.$('input:radio[name=' + this._attribute + ']:checked').val()
                        );
                this.model.set(
                        this._attribute,
                        this.$('input:radio[name=' + this._attribute + ']:checked').val()
                        );
            },
            template: '\
                <div name="colour" class="colourpicker"></div>\
                '
        }
        );

var SignInForm = StaticView.extend(
    {
        template: '\
        <div class="aligned">\
            <div class="group">\
                <label>Username<input name="username" type="text"></input></label>\
            </div>\
            <div class="group">\
                <label>Password<input name="password" type="password"></input></label>\
            </div>\
        </div>\
        ',
        initialize: function() {
            StaticView.prototype.initialize.apply(this, arguments);
            StaticView.prototype.render.apply(this);
            this.on('signin', this.signIn.bind(this));
        },
        signIn: function() {
            var session = new Session;
            session.fetch({
                url: restUri(
                     'auth/session?username=' + this.$('input[name=username]').val() +
                     '&password=' + this.$('input[name=password]').val()
                     ),
                success: (function() {
                    console.log('signed in', session.get('token'));
                    // Store the session token.
                    storage.set('token', session.get('token'));
                    this.trigger('finished');
                }).bind(this)
            });
            return false;
        },
        render: function() {
        }
    }
);

var SignInModal = Modal.extend({
    view: SignInForm,
    buttons: [
        StandardButton.cancel(),
        new ModalButton({
            name: 'signin',
            label: 'Sign In'
        })
    ]
});

/*
 * Push a sign in page to the application whenever a jQuery request receives a
 * 403 error.
 */
$(document).ajaxError(
    function (e, xhr, options, str) {
        // For reasons unknown, a 403 error sometimes triggers this handler
        // before the XHR has the status set.
        if(
            (xhr.readyState == 0 || xhr.status == 403) &&
            !(gApplication.currentModal() instanceof SignInModal)
        ) {
            var m = new SignInModal;
            gApplication.listenTo(
                m,
                'finished',
                function() {
                    gApplication.currentPage().reset();
                }
            );
            gApplication.modal(m);
        }
    }
);

var CurrentUserView = StaticView.extend(
    {
        template: '<%-username%>'
    }
);

var LoadingView = StaticView.extend(
    {
        template: 'Loading'
    }
);

// Map of forecast_weather_main content to an appropriate icon.
var weatherIcon = function(name) {
    return {
        Clear: 'sun',
        Clouds: 'cloud',
        Rain: 'rain'
    }[name];
};

/*
 * Display the currently signed in user.
 */
$(window).load(
    function() {
        var currentUser = new CurrentUser;
        currentUser.fetch();
        (new CurrentUserView({
            el: $('#current-user'),
            model: currentUser
        })).render();
    }
    );
