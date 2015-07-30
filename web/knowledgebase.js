var KnowledgebasePage = PageView.extend(
    {
        pageTitle: 'Knowledge Base',
        template: $('#knowledgebasepage-template').html(),
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this);

            var page = this;

            var families = new FamilyCollection;
            var familiesView = new CheckedCollectionView({
                el: this.$('ul[name=families]'),
                model: families,
                view: StaticView.extend({
                    tagName: 'li',
                    template: '\
                        <label>\
                        <input type="checkbox"></input>\
                        <%-kb_family_cname%> (<%-kb_family_desc%>)\
                        </label>\
                        ',
                    events: {
                        'change input': (function() {
                            this._varietiesView.render();
                        }).bind(this)
                    }
                })
            });
            familiesView.render();
            families.fetch({
                success: familiesView.checkAll.bind(familiesView)
            });

            var edibleParts = new EdiblePartCollection;
            var ediblePartsView = new CheckedCollectionView({
                el: this.$('ul[name=edible]'),
                model: edibleParts,
                view: StaticView.extend({
                    tagName: 'li',
                    template: '\
                        <label>\
                        <input type="checkbox"></input>\
                        <%-kb_edible_part%>\
                        </label>\
                        ',
                    events: {
                        'change input': (function() {
                            this._varietiesView.render();
                        }).bind(this)
                    }
                })
            });
            ediblePartsView.render();
            edibleParts.fetch({
                success: ediblePartsView.checkAll.bind(ediblePartsView)
            });

            var varieties = new VarietyCollection;
            varieties.fetch();

            this._varietiesView = new CollectionView({
                el: this.$('ul[name=varieties]'),
                model: varieties,
                view: StaticView.extend({
                    tagName: 'li',
                    template: '<%-kb_variety_cname%>',
                    events: {
                        click: 'gotoVariety'
                    },
                    attributes: function() {
                        return {
                            style: 'border-left-color: ' +
                                this.model.get('kb_variety_colour') + ';'
                        };
                    },
                    gotoVariety: function() {
                        gApplication.pushPage(
                            new VarietyPage({ model: this.model })
                        );
                    }
                }),
                filter: function(model) {
                    var familyChecked = _(familiesView.checkedIds()).contains(model.get('kb_family_id'));
                    var ediblePartChecked = _(ediblePartsView.checked()).some(
                        function(part) {
                            return part.get('kb_edible_part') == model.get('kb_edible_part');
                        }
                    );
                    // The plant family is checked and (the edible part is
                    // checked, or the plant has no edible part and
                    // 'ornamental' is checked).
                    return familyChecked && (ediblePartChecked || (
                            model.get('kb_edible_part') == null &&
                            page.$('input[name=ornamental]').prop('checked')
                        )
                    );
                }
            });
            this._varietiesView.render();
        },
        events: {
            'change input[name=ornamental]': function() {
                this._varietiesView.render();
            }
        },
        render: function() {
        }
    }
);

var VarietyPage = PageView.extend(
    {
        pageTitle: function() {
            return this.model.get('kb_variety_cname');
        },
        template: $('#varietypage-template').html(),
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            this.model.fetch();
        }
    }
);
