var PalettePage = PageView.extend(
        {
            pageTitle: 'Palette',
            initialize: function() {
                PageView.prototype.initialize.apply(this, arguments);
                PageView.prototype.render.apply(this);
                (new ColourPicker({
                    el: this.$('#colourpicker')
                })).render();
            },
            render: function() {
            }
        }
        );

