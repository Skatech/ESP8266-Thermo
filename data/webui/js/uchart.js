class Bounds {
    /** @param {number} left @param {number} right @param {number} top @param {number} bottom */
    constructor(left, right, top, bottom) {
        this.left = left
        this.right = right
        this.top = top
        this.bottom = bottom
    }

    /** @return {Bounds} */
    makeWeak() {
        this.left = this.bottom = Number.MAX_VALUE
        this.right = this.top = Number.MIN_VALUE
        return this
    }

    /** @param {number} x @param {number} y */
    addPoint(x, y) {
        if (x < this.left) this.left = x
        if (x > this.right) this.right = x
        if (y < this.bottom) this.bottom = y
        if (y > this.top) this.top = y
    }

    /** @param {Bounds} b */
    mergeBounds(b) {
        if (b.left < this.left) this.left = b.left
        if (b.right > this.right) this.right = b.right
        if (b.bottom < this.bottom) this.bottom = b.bottom
        if (b.top > this.top) this.top = b.top
    }

    /** @return {number} */
    get width() {
        return this.right - this.left
    }

    /** @param {number} value */
    set width(value) {
        this.right = this.left + value
    }

    /** @return {number} */
    get height() {
        return this.top - this.bottom
    }

    /** @param {number} value */
    set height(value) {
        this.top = this.bottom + value
    }
    
    /** @param {Bounds} b @return {Bounds} */
    clone(b) {
        return new Bounds(b.left, b.right, b.top, b.bottom)
    }
}

class Chart {
    /** @type {number[]} */ samples = []
    /** @type {number} zero = no limit */ SampleLimit = 0
    /** @type {number} zero = no limit */ RangeLimit = 0

    /** @param {string} color */
    constructor(color) {
        this.color = color
        this.bounds = new Bounds(0,0,0,0)
        this.changed = true
    }

    /** @param {number} limit @return {Chart} Set sample count limit, zero - no limit */
    withSampleLimit(limit) {
        this.SampleLimit = limit
        return this
    }

    /** @param {number} limit @return {Chart} Set x-values range limit, zero - no limit */
    withRangeLimit(limit) {
        this.RangeLimit = limit
        return this
    }

    /** @return {Bounds} */
    getBounds() {
        const samples = this.samples
        const bounds = this.bounds
        if (this.changed) {
            this.changed = false
            bounds.makeWeak()
            for(let i = 0; i < samples.length; ) {
                const x = samples[i++]
                const y = samples[i++]
                bounds.addPoint(x, y)
            }
        }
        return bounds
    }

    addSample(x, y) {
        this.samples.push(x, y)
        this.changed = true
        for  (const limit = this.SampleLimit * 2; limit > 0 && this.samples.length > limit;) {
            this.samples.shift()
        }
        for (const limit = this.RangeLimit; limit > 0 && (x - this.samples[0]) > limit;) {
            this.samples.shift()
            this.samples.shift()
        }
    }

    /** @param {CanvasRenderingContext2D} ctx */
    draw(ctx, scalex, scaley, offsetx, offsety) {
        const samples = this.samples

        ctx.beginPath()
        ctx.strokeStyle = this.color
        ctx.lineWidth = 1
        for(let i = 0; i < samples.length; ) {
            const x = samples[i++] * scalex + offsetx
            const y = samples[i++] * scaley + offsety
            if (i == 0) {
                ctx.moveTo(x, y)
                continue
            }
            ctx.lineTo(x, y)
            ctx.arc(x, y, 1, 0, Math.PI * 2, false)
            ctx.moveTo(x, y)
        }
        ctx.stroke()
    }
}

class ChartPanel {
    offsets = { left: 20, bottom: 20, right: 10, top: 10 }
    defaultBounds = new Bounds(0, 1e-100, 1e-100, 0)
    TickStepX = 20
    TickStepY = 20
    TickCountX = 5
    TickCountY = 5

    /** @param {HTMLCanvasElement} canvas @param {number} width @param {number} height */
    constructor(canvas, width, height) {
        this.canvas = canvas
        canvas.width = width
        canvas.height = height
        /** @type {Chart[]} */ this.charts = []
        this.context = this.canvas.getContext("2d")
    }

    addChart(chart) {
        this.charts.push(chart)
    }

    /** @return {Bounds} */
    getCommonBounds() {
        const bounds = new Bounds(0,0,0,0).makeWeak()
        for(let chart of this.charts) {
            bounds.mergeBounds(chart.getBounds())
        }
        if (bounds.left == Number.MAX_VALUE) {
            bounds.left = this.defaultBounds.left
        }
        if (bounds.bottom == Number.MAX_VALUE) {
            bounds.bottom = this.defaultBounds.bottom
        }
        if (bounds.width < this.defaultBounds.width) {    
            bounds.width = this.defaultBounds.width        
        }
        if (bounds.height < this.defaultBounds.height) {
            bounds.height = this.defaultBounds.height
        }
        return bounds
    }

    /** @param {number} freq @return {number} */
    getBestTickFrequencyFor(freq) {
        const variants = [1, 2, 5] // 0.75-1.5 1.5-3 3.75-7.5
        let multiplier = 1

        while(true) {
            let index = 0
            for (let variant of variants) {
                if (index++ < 1 && variant * multiplier * 0.75 > freq) {
                    break;
                }
                if (variant * multiplier * 1.5 >= freq) {
                    return variant * multiplier
                }
            }
            multiplier *= (index < variants.length) ? 0.1 : 10
        }
    }

    draw() {
        const cnv = this.canvas
        const ctx = this.context

        const OffsetLeft = this.offsets.left, OffsetRight = this.offsets.right
        const OffsetTop = this.offsets.top, OffsetBottom = this.offsets.bottom

        const ChartsWidth = cnv.width - OffsetLeft - OffsetRight
        const ChartsHeight = cnv.height - OffsetBottom - OffsetTop

        const bounds = this.getCommonBounds()
        const scalex = ChartsWidth / Math.max(1, bounds.width)
        const scaley = -ChartsHeight / Math.max(1, bounds.height)
        const offsetx = scalex * -bounds.left + OffsetLeft
        const offsety = scaley * -bounds.bottom + cnv.height - OffsetBottom

        ctx.clearRect(0, 0, cnv.width, cnv.height)

        const tickStepX = this.getBestTickFrequencyFor(bounds.width / this.TickCountX)
        const tickStepY = this.getBestTickFrequencyFor(bounds.height / this.TickCountY)

        ctx.fillStyle = ctx.strokeStyle = "lightgray"
        ctx.lineJoin = "round"
        ctx.lineWidth = 1

        if (tickStepX > 0) {
            ctx.beginPath()
            for (let value = bounds.left - bounds.left % tickStepX; value < bounds.right; value += tickStepX) {
                if (value >= bounds.left) {
                    const x = value * scalex + offsetx
                    const y1 = OffsetTop + ChartsHeight
                    const y2 = OffsetTop
                    ctx.moveTo(x, y1)
                    ctx.lineTo(x, y2)
                    
                    const marker = value.toString()
                    const metric = ctx.measureText(marker)
                    ctx.fillText(marker, x - metric.width / 2, y1 + OffsetBottom / 2)
                }
            }
            ctx.stroke()
        }                
        if (tickStepY > 0) {
            ctx.beginPath()
            for (let value = Math.floor(bounds.bottom - bounds.bottom % tickStepY);
                    value < bounds.top; value += tickStepY) {
                if (value >= bounds.bottom) {
                    const y = value * scaley + offsety
                    const x1 = OffsetLeft
                    const x2 = OffsetLeft + ChartsWidth
                    ctx.moveTo(x1, y)
                    ctx.lineTo(x2, y)

                    const marker = value.toString()
                    const metric = ctx.measureText(marker)
                    ctx.fillText(marker, Math.max(3, x1 - metric.width - 3), y + metric.actualBoundingBoxAscent / 2)
                }
            }
            ctx.stroke()
        }

        for(let chart of this.charts) {
            chart.draw(ctx, scalex, scaley, offsetx, offsety)
        }
    }
}

// console.log("minichart.js - Initialized")
