import QtQuick

Canvas {
    id: particleCanvas
    anchors.fill: parent

    property var particles: []
    property int maxParticles: 12

    Timer {
        interval: 33
        running: true
        repeat: true
        onTriggered: {
            particleCanvas.updateParticles()
            particleCanvas.requestPaint()
        }
    }

    Component.onCompleted: {
        for (var i = 0; i < maxParticles; i++) {
            particles.push(particleCanvas.createParticle())
        }
    }

    function createParticle() {
        return {
            x: Math.random() * width,
            y: Math.random() * height,
            size: 3 + Math.random() * 5,
            opacity: 0.15 + Math.random() * 0.2,
            speed: 0.3 + Math.random() * 0.5,
            angle: Math.random() * Math.PI * 2,
            rotation: Math.random() * Math.PI * 2,
            rotSpeed: (Math.random() - 0.5) * 0.02
        }
    }

    function updateParticles() {
        for (var i = 0; i < particles.length; i++) {
            var p = particles[i]
            p.x += Math.cos(p.angle) * p.speed
            p.y += p.speed * 1.5
            p.rotation += p.rotSpeed
            if (p.y > height + 20) {
                p.y = -20
                p.x = Math.random() * width
            }
            if (p.x < -20 || p.x > width + 20) {
                p.x = Math.random() * width
                p.y = -20
            }
        }
    }

    onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        for (var i = 0; i < particles.length; i++) {
            var p = particles[i]
            ctx.save()
            ctx.translate(p.x, p.y)
            ctx.rotate(p.rotation)
            ctx.globalAlpha = p.opacity
            ctx.fillStyle = "#5EC6A0"
            ctx.beginPath()
            ctx.ellipse(-p.size/2, -p.size/4, p.size, p.size/2)
            ctx.fill()
            ctx.restore()
        }
    }
}
