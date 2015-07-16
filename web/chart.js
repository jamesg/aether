var kelvinToCelsius = function(degK) {
    return degK - 273.15;
};

// Visualisation type for weather symbols.
var symbolVis = {
    enter: function(self, storage, className, data, callbacks) {
        // Select the highest z-index.
        var insertionPoint = xChart.visutils.getInsertionPoint(9);

        storage.container = self._g.selectAll('.temperaturechartsymbols' + className)
            .data(
                data,
                function(d) {
                    return d.className;
                }
            );
        storage.container.enter().insert('g', insertionPoint).attr(
            'class',
            function (d, i) {
                return 'temperaturechartsymbols' + className.replace(/\./g, ' ') +
                    ' color' + i;
            }
        );
        storage.symbols = storage.container.selectAll('g').data(
            function (d) {
                return d.data;
            },
            function (d) {
                return d.x;
            }
        );

        // Create a group for each symbol to translate it independently of
        // scaling.
        storage.symbols.enter()
            .insert('g')
            .attr(
                'transform',
                function(d) {
                    return 'translate(' +
                        (self.xScale(d.x) + self.xScale.rangeBand() / 2) + ' ' +
                        self.yScale(d.y) +
                        ')';
                }
            )
            .insert('use')
            // Use an Open Iconic icon.
            .attr('xlink:href', function(d) { return '#' + d['symbol']; })
            // Centre the symbol on (0, 0) and scale it.
            .attr('transform', 'scale(5) translate(-4 -4)');
    },
    update: function(self, storage, timing) {
        storage.symbols.transition().duration(timing)
            .style('opacity', 1)
            .attr(
                'transform',
                function(d) {
                    // self.xScale and self.yScale will have changed.
                    return 'translate(' +
                        (self.xScale(d.x) + self.xScale.rangeBand() / 2) + ' ' +
                        self.yScale(d.y) +
                        ')';
                }
            );
    },
    exit: function(self, storage, timing) {
        storage.symbols.exit()
            .transition().duration(timing)
            .style('opacity', 0);
    },
    destroy: function(self, storage, timing) {
        storage.symbols.transition().duration(timing)
            .style('opacity', 0)
            .remove();
    }
};

xChart.setVis('symbolVis', symbolVis);

var Chart = function(options) {
    this._chartId = options['id'];
    this._chartType = options['type'];
    this.on('setData', this.render.bind(this));
};

Chart.prototype.setData = function(data) {
    this._data = data;
    this.trigger('setData');
};

Chart.prototype.render = function() {
    if(_.has(this, '_chart'))
        this._chart.setData(this._data);
    else
        this._chart = new xChart(this._chartType, this._data, '#' + this._chartId);
};

_.extend(Chart.prototype, Backbone.Events);
