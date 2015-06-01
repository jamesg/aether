var HomePage = PageView.extend(
    {
        pageTitle: 'Home',
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            this.render();
        },
        template: $('#homepage-template').html(),
        events: {
            'click button[name=show-modal]': 'showModal'
        },
        showModal: function() {
            var m = new Modal({
                buttons: [
                    ModalButton.cancel(),
                    ModalButton.create(function() { console.log('create'); m.finish(); })
                ],
                view: StaticView.extend({ template: 'Modal' })
            });
            gApplication.modal(m);
        }
    }
    );

