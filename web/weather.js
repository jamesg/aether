var WeatherPage = PageView.extend(
        {
            pageTitle: 'Weather',
            template: $('#weatherpage-template').html(),
            initialize: function() {
                PageView.prototype.initialize.apply(this, arguments);
                PageView.prototype.render.apply(this);
            },
            render: function() {
            }
        }
        );

