import QtQuick
import QtQuick.Controls
import CodeFarm

Rectangle {
    id: root
    width: parent ? parent.width : 0
    height: parent ? parent.height : 0
    color: Theme.bgMain

    // Layout constants
    readonly property int nodeSpacing: 170
    readonly property int trackStartX: 250
    readonly property real trackY: 430
    readonly property int nodeR: 40
    readonly property int totalContentWidth: trackStartX + 19 * nodeSpacing + 300

    // Level data model
    ListModel { id: listModel }

    property int currentLevelIdx: 0

    function loadModel() {
        listModel.clear()
        var levels = appVm.levels
        for (var i = 0; i < levels.length; i++) {
            var lv = levels[i]
            listModel.append({
                id: lv.id, name: lv.name, stars: lv.stars,
                unlocked: lv.unlocked, gridW: lv.gridW, gridH: lv.gridH,
                bestTime: lv.bestTime,
                nodeX: trackStartX + i * nodeSpacing
            })
        }
        // Find current level to center on
        currentLevelIdx = 0
        for (var j = 0; j < listModel.count; j++) {
            var item = listModel.get(j)
            if (item.stars === 0 && item.unlocked) { currentLevelIdx = j; break }
            if (item.stars > 0) currentLevelIdx = j + 1
        }
        if (currentLevelIdx >= listModel.count) currentLevelIdx = listModel.count - 1
    }

    Component.onCompleted: loadModel()

    onVisibleChanged: {
        if (visible) {
            loadModel()
            for (var k = 0; k < nodeRepeater.count; k++) {
                var child = nodeRepeater.itemAt(k)
                if (child && child.nodeCanvas) child.nodeCanvas.requestPaint()
            }
            trackCanvas.requestPaint()
            var cx = trackStartX + currentLevelIdx * nodeSpacing
            flick.contentX = Math.max(0, cx - root.width / 2)
        }
    }

    // =====================================================================
    // 31 Icon drawing functions
    // =====================================================================

    // 1: 单株小麦（初次收割）
    function drawIcon01(ctx, cx, cy) {
        ctx.strokeStyle = "#6B8040"; ctx.lineWidth = 2.5; ctx.lineCap = "round"
        ctx.beginPath(); ctx.moveTo(cx, cy+18); ctx.lineTo(cx, cy-10); ctx.stroke()
        ctx.fillStyle = "#E8C840"
        ctx.beginPath(); ctx.ellipse(cx, cy-15, 4, 7, -0.15, 0, Math.PI*2); ctx.fill()
        ctx.beginPath(); ctx.ellipse(cx-5, cy-12, 3.5, 6, -0.3, 0, Math.PI*2); ctx.fill()
        ctx.beginPath(); ctx.ellipse(cx+5, cy-12, 3.5, 6, 0.3, 0, Math.PI*2); ctx.fill()
    }

    // 2: 箭头+小麦（移动起步）
    function drawIcon02(ctx, cx, cy) {
        // Arrow
        ctx.strokeStyle = "#5AAFCF"; ctx.lineWidth = 2.5; ctx.lineCap = "round"
        ctx.beginPath(); ctx.moveTo(cx-16, cy); ctx.lineTo(cx+10, cy); ctx.stroke()
        ctx.beginPath(); ctx.moveTo(cx+4, cy-6); ctx.lineTo(cx+10, cy); ctx.lineTo(cx+4, cy+6); ctx.stroke()
        // Small wheat
        ctx.strokeStyle = "#6B8040"; ctx.lineWidth = 1.5
        ctx.beginPath(); ctx.moveTo(cx-8, cy+12); ctx.lineTo(cx-8, cy+2); ctx.stroke()
        ctx.fillStyle = "#E8C840"
        ctx.beginPath(); ctx.ellipse(cx-8, cy-2, 3, 5, 0, 0, Math.PI*2); ctx.fill()
    }

    // 3: 一排小麦（直线收割）
    function drawIcon03(ctx, cx, cy) {
        for (var i = -1; i <= 1; i++) {
            var wx = cx + i * 12
            ctx.strokeStyle = "#6B8040"; ctx.lineWidth = 1.5; ctx.lineCap = "round"
            ctx.beginPath(); ctx.moveTo(wx, cy+13); ctx.lineTo(wx, cy-5); ctx.stroke()
            ctx.fillStyle = "#E8C840"
            ctx.beginPath(); ctx.ellipse(wx, cy-9, 3, 5, 0, 0, Math.PI*2); ctx.fill()
            ctx.beginPath(); ctx.ellipse(wx-2.5, cy-7, 2.5, 4, -0.2, 0, Math.PI*2); ctx.fill()
        }
    }

    // 4: 十字箭头（田字格）
    function drawIcon04(ctx, cx, cy) {
        ctx.strokeStyle = "#5AAFCF"; ctx.lineWidth = 2.5; ctx.lineCap = "round"
        // Horizontal arrow
        ctx.beginPath(); ctx.moveTo(cx-16, cy); ctx.lineTo(cx+16, cy); ctx.stroke()
        ctx.beginPath(); ctx.moveTo(cx+10, cy-6); ctx.lineTo(cx+16, cy); ctx.lineTo(cx+10, cy+6); ctx.stroke()
        // Vertical arrow
        ctx.beginPath(); ctx.moveTo(cx, cy-16); ctx.lineTo(cx, cy+16); ctx.stroke()
        ctx.beginPath(); ctx.moveTo(cx-6, cy-10); ctx.lineTo(cx, cy-16); ctx.lineTo(cx+6, cy-10); ctx.stroke()
    }

    // 5: 水壶（认识浇水）
    function drawIcon05(ctx, cx, cy) {
        ctx.fillStyle = "#5AAFCF"
        ctx.beginPath(); ctx.moveTo(cx-10, cy-3); ctx.lineTo(cx+8, cy-3); ctx.lineTo(cx+12, cy+8); ctx.lineTo(cx-14, cy+8); ctx.closePath(); ctx.fill()
        ctx.fillStyle = "#4A9ABF"; ctx.fillRect(cx-12, cy-5, 22, 3)
        ctx.strokeStyle = "#5AAFCF"; ctx.lineWidth = 2.5; ctx.lineCap = "round"
        ctx.beginPath(); ctx.moveTo(cx+8, cy-1); ctx.lineTo(cx+15, cy-10); ctx.stroke()
        ctx.fillStyle = "#5AAFCF"; ctx.beginPath(); ctx.arc(cx+15, cy-12, 3.5, 0, Math.PI*2); ctx.fill()
        ctx.fillStyle = "#8AD4EF"
        ctx.beginPath(); ctx.arc(cx+19, cy-5, 1.5, 0, Math.PI*2); ctx.fill()
        ctx.beginPath(); ctx.arc(cx+17, cy, 1.2, 0, Math.PI*2); ctx.fill()
    }

    // 6: 水壶+植物（浇水实践）
    function drawIcon06(ctx, cx, cy) {
        // Water drops
        ctx.fillStyle = "#5AAFCF"
        ctx.beginPath(); ctx.ellipse(cx-10, cy-8, 3, 5, 0, 0, Math.PI*2); ctx.fill()
        ctx.beginPath(); ctx.ellipse(cx+10, cy-8, 3, 5, 0, 0, Math.PI*2); ctx.fill()
        // Plant below
        ctx.strokeStyle = "#6B8040"; ctx.lineWidth = 2; ctx.lineCap = "round"
        ctx.beginPath(); ctx.moveTo(cx, cy+15); ctx.lineTo(cx, cy+2); ctx.stroke()
        ctx.fillStyle = "#7AB356"
        ctx.beginPath(); ctx.moveTo(cx, cy+5); ctx.bezierCurveTo(cx-6, cy+2, cx-10, cy-1, cx-7, cy+4); ctx.fill()
        ctx.beginPath(); ctx.moveTo(cx, cy+8); ctx.bezierCurveTo(cx+6, cy+5, cx+10, cy+2, cx+7, cy+7); ctx.fill()
    }

    // 7: 移动练习 — 蛇形路径
    function drawIcon07(ctx, cx, cy) {
        ctx.strokeStyle = "#5AAFCF"; ctx.lineWidth = 2; ctx.lineCap = "round"; ctx.lineJoin = "round"
        ctx.beginPath()
        ctx.moveTo(cx-14, cy-12); ctx.lineTo(cx+14, cy-12); ctx.lineTo(cx+14, cy); ctx.lineTo(cx-14, cy); ctx.lineTo(cx-14, cy+12); ctx.lineTo(cx+14, cy+12)
        ctx.stroke()
        // Dots at turns
        ctx.fillStyle = "#E8C840"
        ctx.beginPath(); ctx.arc(cx-14, cy-12, 3, 0, Math.PI*2); ctx.fill()
        ctx.beginPath(); ctx.arc(cx+14, cy, 3, 0, Math.PI*2); ctx.fill()
        ctx.beginPath(); ctx.arc(cx-14, cy+12, 3, 0, Math.PI*2); ctx.fill()
    }

    // 8: 眼睛/雷达（感知环境）
    function drawIcon08(ctx, cx, cy) {
        ctx.strokeStyle = "#5AAFCF"; ctx.lineWidth = 2
        ctx.beginPath(); ctx.ellipse(cx, cy, 16, 10, 0, 0, Math.PI*2); ctx.stroke()
        ctx.fillStyle = "#5AAFCF"; ctx.beginPath(); ctx.arc(cx, cy, 5, 0, Math.PI*2); ctx.fill()
        ctx.fillStyle = "#FFFFFF"; ctx.beginPath(); ctx.arc(cx-1.5, cy-1.5, 2, 0, Math.PI*2); ctx.fill()
    }

    // 9: 循环箭头（for 循环）
    function drawIcon09(ctx, cx, cy) {
        ctx.strokeStyle = "#E88030"; ctx.lineWidth = 2.5; ctx.lineCap = "round"
        // Circular arrow
        ctx.beginPath(); ctx.arc(cx, cy, 14, -0.5, Math.PI*1.5); ctx.stroke()
        // Arrowhead
        var ax = cx + 14*Math.cos(Math.PI*1.5), ay = cy + 14*Math.sin(Math.PI*1.5)
        ctx.beginPath(); ctx.moveTo(ax-5, ay-4); ctx.lineTo(ax, ay); ctx.lineTo(ax+5, ay-2); ctx.stroke()
        // "for" text
        ctx.fillStyle = "#E88030"; ctx.font = "bold 10px monospace"
        ctx.fillText("for", cx-8, cy+4)
    }

    // 10: 循环箭头+小麦（循环收割）
    function drawIcon10(ctx, cx, cy) {
        // Mini loop arrow
        ctx.strokeStyle = "#E88030"; ctx.lineWidth = 2; ctx.lineCap = "round"
        ctx.beginPath(); ctx.arc(cx-8, cy, 8, -0.5, Math.PI*1.5); ctx.stroke()
        // Wheat
        ctx.strokeStyle = "#6B8040"; ctx.lineWidth = 1.5
        ctx.beginPath(); ctx.moveTo(cx+8, cy+12); ctx.lineTo(cx+8, cy-2); ctx.stroke()
        ctx.fillStyle = "#E8C840"
        ctx.beginPath(); ctx.ellipse(cx+8, cy-6, 3, 5, 0, 0, Math.PI*2); ctx.fill()
    }

    // 11: 铲子+种子（耕种入门）
    function drawIcon11(ctx, cx, cy) {
        // Shovel
        ctx.fillStyle = "#8B7355"
        ctx.fillRect(cx-2, cy-8, 4, 16)
        ctx.fillStyle = "#A0A0A0"
        ctx.beginPath(); ctx.moveTo(cx-8, cy+8); ctx.lineTo(cx+8, cy+8); ctx.lineTo(cx+5, cy+18); ctx.lineTo(cx-5, cy+18); ctx.closePath(); ctx.fill()
        // Seed
        ctx.fillStyle = "#E8C840"
        ctx.beginPath(); ctx.ellipse(cx+12, cy-6, 3, 4, 0.3, 0, Math.PI*2); ctx.fill()
    }

    // 12: 水壶+问号（条件浇水）
    function drawIcon12(ctx, cx, cy) {
        // Water drop
        ctx.fillStyle = "#5AAFCF"
        ctx.beginPath(); ctx.ellipse(cx-8, cy+2, 5, 8, 0, 0, Math.PI*2); ctx.fill()
        // Question mark
        ctx.fillStyle = "#E8C840"; ctx.font = "bold 18px sans-serif"
        ctx.fillText("?", cx+4, cy+8)
    }

    // 13: 肥料袋（施肥加速）
    function drawIcon13(ctx, cx, cy) {
        ctx.fillStyle = "#8B6914"
        ctx.beginPath(); ctx.moveTo(cx-12, cy-6); ctx.lineTo(cx+12, cy-6); ctx.lineTo(cx+10, cy+14); ctx.lineTo(cx-10, cy+14); ctx.closePath(); ctx.fill()
        ctx.fillStyle = "#A07818"; ctx.fillRect(cx-14, cy-8, 28, 4)
        ctx.fillStyle = "#FFFFFF"; ctx.font = "bold 10px sans-serif"
        ctx.fillText("NPK", cx-11, cy+8)
        // Sparkle
        ctx.strokeStyle = "#F0D060"; ctx.lineWidth = 1.5
        ctx.beginPath(); ctx.moveTo(cx+10, cy-14); ctx.lineTo(cx+10, cy-8); ctx.stroke()
        ctx.beginPath(); ctx.moveTo(cx+7, cy-11); ctx.lineTo(cx+13, cy-11); ctx.stroke()
    }

    // 14: 时钟（时间竞速）
    function drawIcon14(ctx, cx, cy) {
        ctx.strokeStyle = "#E88030"; ctx.lineWidth = 2.5
        ctx.beginPath(); ctx.arc(cx, cy, 14, 0, Math.PI*2); ctx.stroke()
        // Clock hands
        ctx.lineWidth = 2; ctx.lineCap = "round"
        ctx.beginPath(); ctx.moveTo(cx, cy); ctx.lineTo(cx, cy-10); ctx.stroke()
        ctx.beginPath(); ctx.moveTo(cx, cy); ctx.lineTo(cx+8, cy+2); ctx.stroke()
        // Center dot
        ctx.fillStyle = "#E88030"; ctx.beginPath(); ctx.arc(cx, cy, 2, 0, Math.PI*2); ctx.fill()
    }

    // 15: 玉米+虫子（玉米耐心）
    function drawIcon15(ctx, cx, cy) {
        // Corn
        ctx.fillStyle = "#F0D040"
        ctx.beginPath(); ctx.moveTo(cx-6, cy-10); ctx.bezierCurveTo(cx-9, cy-7, cx-8, cy+11, cx-5, cy+16); ctx.lineTo(cx+5, cy+16); ctx.bezierCurveTo(cx+8, cy+11, cx+9, cy-7, cx+6, cy-10); ctx.closePath(); ctx.fill()
        ctx.fillStyle = "#6B8040"
        ctx.beginPath(); ctx.moveTo(cx-6, cy-10); ctx.bezierCurveTo(cx-14, cy-6, cx-14, cy+2, cx-8, cy-4); ctx.fill()
        ctx.beginPath(); ctx.moveTo(cx+6, cy-10); ctx.bezierCurveTo(cx+14, cy-6, cx+14, cy+2, cx+8, cy-4); ctx.fill()
        // Bug
        ctx.fillStyle = "#D94F4F"
        ctx.beginPath(); ctx.arc(cx+14, cy+10, 4, 0, Math.PI*2); ctx.fill()
        ctx.strokeStyle = "#000"; ctx.lineWidth = 0.8
        ctx.beginPath(); ctx.moveTo(cx+12, cy+10); ctx.lineTo(cx+16, cy+10); ctx.stroke()
    }

    // 16: 喷雾器（虫害巡逻）
    function drawIcon16(ctx, cx, cy) {
        // Spray bottle
        ctx.fillStyle = "#4A9ABF"
        ctx.beginPath(); ctx.roundedRect(cx-8, cy-4, 16, 18, 3, 3); ctx.fill()
        ctx.fillStyle = "#3A8AAF"; ctx.fillRect(cx-3, cy-10, 6, 8)
        // Nozzle
        ctx.fillStyle = "#666"
        ctx.fillRect(cx+3, cy-12, 8, 3)
        // Spray mist
        ctx.fillStyle = "rgba(138, 212, 239, 0.6)"
        ctx.beginPath(); ctx.arc(cx+14, cy-10, 3, 0, Math.PI*2); ctx.fill()
        ctx.beginPath(); ctx.arc(cx+16, cy-6, 2, 0, Math.PI*2); ctx.fill()
        ctx.beginPath(); ctx.arc(cx+12, cy-14, 2, 0, Math.PI*2); ctx.fill()
    }

    // 17: while 符号（while 循环）
    function drawIcon17(ctx, cx, cy) {
        ctx.strokeStyle = "#A060C0"; ctx.lineWidth = 2.5; ctx.lineCap = "round"
        // Infinity-like loop
        ctx.beginPath(); ctx.arc(cx-7, cy, 9, 0, Math.PI*2); ctx.stroke()
        ctx.beginPath(); ctx.arc(cx+7, cy, 9, 0, Math.PI*2); ctx.stroke()
        // "w" label
        ctx.fillStyle = "#A060C0"; ctx.font = "bold 10px monospace"
        ctx.fillText("w", cx-3, cy+4)
    }

    // 18: 循环+喷雾器（循环巡逻）
    function drawIcon18(ctx, cx, cy) {
        // Loop arrow
        ctx.strokeStyle = "#A060C0"; ctx.lineWidth = 2; ctx.lineCap = "round"
        ctx.beginPath(); ctx.arc(cx-8, cy-2, 10, -0.5, Math.PI*1.3); ctx.stroke()
        // Spray
        ctx.fillStyle = "#4A9ABF"
        ctx.beginPath(); ctx.roundedRect(cx+4, cy-4, 10, 14, 2, 2); ctx.fill()
        ctx.fillStyle = "rgba(138, 212, 239, 0.5)"
        ctx.beginPath(); ctx.arc(cx+16, cy-6, 2.5, 0, Math.PI*2); ctx.fill()
    }

    // 19: 多种植物（混合种植）
    function drawIcon19(ctx, cx, cy) {
        // Wheat
        ctx.strokeStyle = "#6B8040"; ctx.lineWidth = 1.5; ctx.lineCap = "round"
        ctx.beginPath(); ctx.moveTo(cx-12, cy+12); ctx.lineTo(cx-12, cy-2); ctx.stroke()
        ctx.fillStyle = "#E8C840"; ctx.beginPath(); ctx.ellipse(cx-12, cy-6, 3, 5, 0, 0, Math.PI*2); ctx.fill()
        // Carrot
        ctx.fillStyle = "#E88030"
        ctx.beginPath(); ctx.moveTo(cx, cy-4); ctx.lineTo(cx-3, cy+12); ctx.lineTo(cx+3, cy+12); ctx.closePath(); ctx.fill()
        ctx.fillStyle = "#7AB356"
        ctx.beginPath(); ctx.moveTo(cx, cy-4); ctx.bezierCurveTo(cx-3, cy-8, cx-6, cy-10, cx-4, cy-6); ctx.fill()
        // Tomato
        ctx.fillStyle = "#CF5A5A"; ctx.beginPath(); ctx.arc(cx+12, cy+4, 7, 0, Math.PI*2); ctx.fill()
        ctx.fillStyle = "#6B8040"; ctx.lineWidth = 1.5
        ctx.beginPath(); ctx.moveTo(cx+12, cy-3); ctx.bezierCurveTo(cx+10, cy-6, cx+14, cy-8, cx+13, cy-4); ctx.stroke()
    }

    // 20: f(x) 函数符号（定义函数）
    function drawIcon20(ctx, cx, cy) {
        ctx.fillStyle = "#A060C0"; ctx.font = "bold 16px monospace"
        ctx.fillText("f(x)", cx-14, cy+6)
        // Brace
        ctx.strokeStyle = "#A060C0"; ctx.lineWidth = 2; ctx.lineCap = "round"
        ctx.beginPath(); ctx.moveTo(cx+16, cy-10); ctx.lineTo(cx+20, cy-10); ctx.lineTo(cx+20, cy+10); ctx.lineTo(cx+16, cy+10); ctx.stroke()
    }

    // 21: 函数+网格（函数复用）
    function drawIcon21(ctx, cx, cy) {
        // Grid
        ctx.strokeStyle = "rgba(255,255,255,0.3)"; ctx.lineWidth = 1
        for (var gx = -12; gx <= 12; gx += 8) {
            ctx.beginPath(); ctx.moveTo(cx+gx, cy-12); ctx.lineTo(cx+gx, cy+12); ctx.stroke()
        }
        for (var gy = -12; gy <= 12; gy += 8) {
            ctx.beginPath(); ctx.moveTo(cx-12, cy+gy); ctx.lineTo(cx+12, cy+gy); ctx.stroke()
        }
        // "f" overlay
        ctx.fillStyle = "#A060C0"; ctx.font = "bold 14px monospace"
        ctx.fillText("f", cx-4, cy+6)
    }

    // 22: 盾牌（虫害防御）
    function drawIcon22(ctx, cx, cy) {
        ctx.fillStyle = "#4A8C3A"
        ctx.beginPath()
        ctx.moveTo(cx, cy-16); ctx.lineTo(cx+14, cy-8); ctx.lineTo(cx+14, cy+4);
        ctx.bezierCurveTo(cx+14, cy+14, cx, cy+18, cx, cy+18);
        ctx.bezierCurveTo(cx, cy+18, cx-14, cy+14, cx-14, cy+4);
        ctx.lineTo(cx-14, cy-8); ctx.closePath(); ctx.fill()
        ctx.strokeStyle = "#5A9C4A"; ctx.lineWidth = 2; ctx.stroke()
        // Check mark
        ctx.strokeStyle = "#FFFFFF"; ctx.lineWidth = 2.5; ctx.lineCap = "round"; ctx.lineJoin = "round"
        ctx.beginPath(); ctx.moveTo(cx-5, cy+1); ctx.lineTo(cx-1, cy+5); ctx.lineTo(cx+6, cy-4); ctx.stroke()
    }

    // 23: 与或非符号（布尔逻辑）
    function drawIcon23(ctx, cx, cy) {
        ctx.fillStyle = "#E8C840"; ctx.font = "bold 11px monospace"
        ctx.fillText("AND", cx-13, cy-6)
        ctx.fillStyle = "#5AAFCF"
        ctx.fillText(" OR", cx-13, cy+6)
        ctx.fillStyle = "#E88030"
        ctx.fillText("NOT", cx-10, cy+18)
    }

    // 24: 电池（资源管理）
    function drawIcon24(ctx, cx, cy) {
        // Battery body
        ctx.strokeStyle = "#5AAFCF"; ctx.lineWidth = 2
        ctx.beginPath(); ctx.roundedRect(cx-14, cy-8, 28, 16, 3, 3); ctx.stroke()
        // Battery tip
        ctx.fillStyle = "#5AAFCF"
        ctx.fillRect(cx+14, cy-4, 4, 8)
        // Fill level
        ctx.fillStyle = "#4CAF50"
        ctx.fillRect(cx-11, cy-5, 18, 10)
        // Lightning
        ctx.fillStyle = "#F0D060"
        ctx.beginPath(); ctx.moveTo(cx+2, cy-6); ctx.lineTo(cx-3, cy+1); ctx.lineTo(cx+1, cy+1); ctx.lineTo(cx-2, cy+8); ctx.lineTo(cx+5, cy-1); ctx.lineTo(cx+1, cy-1); ctx.closePath(); ctx.fill()
    }

    // 25: 秒表（计时优化）
    function drawIcon25(ctx, cx, cy) {
        ctx.strokeStyle = "#E88030"; ctx.lineWidth = 2.5
        ctx.beginPath(); ctx.arc(cx, cy+2, 14, 0, Math.PI*2); ctx.stroke()
        // Top button
        ctx.fillStyle = "#E88030"; ctx.fillRect(cx-3, cy-14, 6, 5)
        // Hands
        ctx.lineWidth = 2; ctx.lineCap = "round"
        ctx.beginPath(); ctx.moveTo(cx, cy+2); ctx.lineTo(cx, cy-6); ctx.stroke()
        ctx.beginPath(); ctx.moveTo(cx, cy+2); ctx.lineTo(cx+7, cy+6); ctx.stroke()
        // Center
        ctx.fillStyle = "#E88030"; ctx.beginPath(); ctx.arc(cx, cy+2, 2, 0, Math.PI*2); ctx.fill()
    }

    // 26: break 符号（高级控制）
    function drawIcon26(ctx, cx, cy) {
        // Loop arrow with break
        ctx.strokeStyle = "#A060C0"; ctx.lineWidth = 2.5; ctx.lineCap = "round"
        ctx.beginPath(); ctx.arc(cx, cy, 12, 0, Math.PI*1.6); ctx.stroke()
        // X at break point
        var bx = cx + 12*Math.cos(Math.PI*1.6), by = cy + 12*Math.sin(Math.PI*1.6)
        ctx.strokeStyle = "#D94F4F"; ctx.lineWidth = 3
        ctx.beginPath(); ctx.moveTo(bx-4, by-4); ctx.lineTo(bx+4, by+4); ctx.stroke()
        ctx.beginPath(); ctx.moveTo(bx+4, by-4); ctx.lineTo(bx-4, by+4); ctx.stroke()
    }

    // 27: 迷宫/路径（路径规划）
    function drawIcon27(ctx, cx, cy) {
        ctx.strokeStyle = "#5AAFCF"; ctx.lineWidth = 2; ctx.lineCap = "round"; ctx.lineJoin = "round"
        ctx.beginPath()
        ctx.moveTo(cx-14, cy-12); ctx.lineTo(cx-4, cy-12); ctx.lineTo(cx-4, cy-4);
        ctx.lineTo(cx+6, cy-4); ctx.lineTo(cx+6, cy+4); ctx.lineTo(cx-4, cy+4);
        ctx.lineTo(cx-4, cy+12); ctx.lineTo(cx+14, cy+12)
        ctx.stroke()
        // Start dot
        ctx.fillStyle = "#4CAF50"; ctx.beginPath(); ctx.arc(cx-14, cy-12, 3, 0, Math.PI*2); ctx.fill()
        // End dot
        ctx.fillStyle = "#E88030"; ctx.beginPath(); ctx.arc(cx+14, cy+12, 3, 0, Math.PI*2); ctx.fill()
    }

    // 28: 向日葵
    function drawIcon28(ctx, cx, cy) {
        ctx.strokeStyle = "#5A9336"; ctx.lineWidth = 2.5; ctx.lineCap = "round"
        ctx.beginPath(); ctx.moveTo(cx, cy+8); ctx.lineTo(cx, cy+20); ctx.stroke()
        ctx.fillStyle = "#7AB356"
        ctx.beginPath(); ctx.moveTo(cx, cy+15); ctx.bezierCurveTo(cx+3, cy+13, cx+10, cy+12, cx+8, cy+16); ctx.bezierCurveTo(cx+6, cy+18, cx+3, cy+18, cx, cy+15); ctx.fill()
        ctx.fillStyle = "#F0C030"
        for (var pi = 0; pi < 10; pi++) {
            var angle = pi * Math.PI * 2 / 10
            var px = cx + Math.cos(angle) * 12, py = cy + Math.sin(angle) * 12
            ctx.save(); ctx.translate(px, py); ctx.rotate(angle)
            ctx.beginPath(); ctx.ellipse(0, 0, 3.5, 7, 0, 0, Math.PI*2); ctx.fill()
            ctx.restore()
        }
        ctx.fillStyle = "#6B4020"; ctx.beginPath(); ctx.arc(cx, cy, 7, 0, Math.PI*2); ctx.fill()
    }

    // 29: 混合田地（多作物挑战）
    function drawIcon29(ctx, cx, cy) {
        // 4 colored quadrants
        ctx.fillStyle = "#E8C840"; ctx.fillRect(cx-14, cy-14, 14, 14) // wheat
        ctx.fillStyle = "#E88030"; ctx.fillRect(cx, cy-14, 14, 14)     // carrot
        ctx.fillStyle = "#CF5A5A"; ctx.fillRect(cx-14, cy, 14, 14)     // tomato
        ctx.fillStyle = "#F0D040"; ctx.fillRect(cx, cy, 14, 14)        // corn
        // Grid lines
        ctx.strokeStyle = "rgba(0,0,0,0.3)"; ctx.lineWidth = 1
        ctx.beginPath(); ctx.moveTo(cx, cy-14); ctx.lineTo(cx, cy+14); ctx.stroke()
        ctx.beginPath(); ctx.moveTo(cx-14, cy); ctx.lineTo(cx+14, cy); ctx.stroke()
    }

    // 30: 大农场鸟瞰
    function drawIcon30(ctx, cx, cy) {
        // Grid of small colored dots
        var colors = ["#E8C840", "#E88030", "#CF5A5A", "#F0D040", "#F0C030"]
        for (var gy = 0; gy < 3; gy++) {
            for (var gx = 0; gx < 4; gx++) {
                ctx.fillStyle = colors[(gy*4+gx) % 5]
                ctx.beginPath(); ctx.arc(cx-12+gx*8, cy-8+gy*8, 3, 0, Math.PI*2); ctx.fill()
            }
        }
    }

    // 31: 金色星星（终极挑战）
    function drawIcon31(ctx, cx, cy) {
        ctx.fillStyle = "#F5C518"
        var spikes = 5, outerR = 16, innerR = 7
        ctx.beginPath()
        for (var si = 0; si < spikes * 2; si++) {
            var r = si % 2 === 0 ? outerR : innerR
            var a = si * Math.PI / spikes - Math.PI / 2
            var sx = cx + Math.cos(a) * r, sy = cy + Math.sin(a) * r
            if (si === 0) ctx.moveTo(sx, sy)
            else ctx.lineTo(sx, sy)
        }
        ctx.closePath(); ctx.fill()
        ctx.strokeStyle = "#D4A010"; ctx.lineWidth = 1.5; ctx.stroke()
    }

    function drawNodeIcon(ctx, idx, cx, cy) {
        // 20 levels: icon mapping matches level themes
        // 1:初次耕种 2:连种三格 3:田字耕种 4:四方耕种 5:认识胡萝卜
        // 6:感知检查 7:for循环 8:循环耕田 9:定义函数 10:施肥加速
        // 11:多作物管理 12:玉米与虫害 13:巡逻防线 14:布尔逻辑 15:计时挑战
        // 16:高级控制 17:路径规划 18:多作物限时 19:全面挑战 20:终极挑战
        switch (idx) {
        case 0: drawIcon01(ctx, cx, cy); break   // 初次耕种 — 单株小麦
        case 1: drawIcon03(ctx, cx, cy); break   // 连种三格 — 一排小麦
        case 2: drawIcon04(ctx, cx, cy); break   // 田字耕种 — 十字箭头
        case 3: drawIcon07(ctx, cx, cy); break   // 四方耕种 — 蛇形路径
        case 4: drawIcon05(ctx, cx, cy); break   // 认识胡萝卜 — 水壶
        case 5: drawIcon08(ctx, cx, cy); break   // 感知检查 — 眼睛/雷达
        case 6: drawIcon09(ctx, cx, cy); break   // for 循环 — 循环箭头
        case 7: drawIcon10(ctx, cx, cy); break   // 循环耕田 — 循环+小麦
        case 8: drawIcon20(ctx, cx, cy); break   // 定义函数 — f(x)
        case 9: drawIcon13(ctx, cx, cy); break   // 施肥加速 — 肥料袋
        case 10: drawIcon19(ctx, cx, cy); break  // 多作物管理 — 多种植物
        case 11: drawIcon15(ctx, cx, cy); break  // 玉米与虫害 — 玉米+虫子
        case 12: drawIcon16(ctx, cx, cy); break  // 巡逻防线 — 喷雾器
        case 13: drawIcon23(ctx, cx, cy); break  // 布尔逻辑 — AND OR NOT
        case 14: drawIcon14(ctx, cx, cy); break  // 计时挑战 — 时钟
        case 15: drawIcon26(ctx, cx, cy); break  // 高级控制 — break符号
        case 16: drawIcon27(ctx, cx, cy); break  // 路径规划 — 迷宫路径
        case 17: drawIcon29(ctx, cx, cy); break  // 多作物限时 — 混合田地
        case 18: drawIcon30(ctx, cx, cy); break  // 全面挑战 — 大农场鸟瞰
        case 19: drawIcon31(ctx, cx, cy); break  // 终极挑战 — 金色星星
        }
    }

    // =====================================================================
    // Top progress bar (dynamic count)
    // =====================================================================
    Item {
        id: progressBar
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 40
        width: Math.min(parent.width - 200, 800)
        height: 20
        z: 100

        // Line
        Canvas {
            anchors.fill: parent
            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                ctx.strokeStyle = "rgba(255,255,255,0.3)"
                ctx.lineWidth = 1.5
                ctx.beginPath()
                ctx.moveTo(0, height/2)
                ctx.lineTo(width, height/2)
                ctx.stroke()
            }
            Component.onCompleted: requestPaint()
        }

        // Dots (dynamic count)
        Repeater {
            model: listModel.count
            Item {
                x: listModel.count > 1 ? index * (progressBar.width / (listModel.count - 1)) - 4 : progressBar.width / 2 - 4
                y: progressBar.height / 2 - 4
                width: 8
                height: 8

                property bool completed: listModel.count > index && listModel.get(index).stars > 0
                property bool isCurrent: listModel.count > index && listModel.get(index).stars === 0 && listModel.get(index).unlocked

                Rectangle {
                    anchors.centerIn: parent
                    width: 8
                    height: 8
                    radius: 4
                    color: completed ? "#F5C518" : (isCurrent ? "#FFFFFF" : "rgba(255,255,255,0.3)")
                    border.width: isCurrent ? 2 : 0
                    border.color: "#FFFFFF"
                }
                // Current indicator triangle
                Canvas {
                    anchors.horizontalCenter: parent.horizontalCenter
                    y: parent.height + 2
                    width: 8
                    height: 5
                    visible: isCurrent
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        ctx.fillStyle = "#FFFFFF"
                        ctx.beginPath()
                        ctx.moveTo(0, 0)
                        ctx.lineTo(width, 0)
                        ctx.lineTo(width/2, height)
                        ctx.closePath()
                        ctx.fill()
                    }
                    Component.onCompleted: requestPaint()
                }
            }
        }
    }

    // =====================================================================
    // Horizontal scrollable area
    // =====================================================================
    Flickable {
        id: flick
        anchors.fill: parent
        contentWidth: totalContentWidth
        contentHeight: height
        flickableDirection: Flickable.HorizontalFlick
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        // Horizontal track line
        Canvas {
            id: trackCanvas
            x: 0
            y: 0
            width: totalContentWidth
            height: root.height
            z: 0

            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)

                // ---- Scenic background ----
                ctx.globalAlpha = 0.06
                ctx.fillStyle = "#5C6B28"
                ctx.beginPath()
                ctx.moveTo(0, root.height * 0.75)
                for (var hx = 0; hx <= totalContentWidth; hx += 8)
                    ctx.lineTo(hx, root.height * 0.75 + Math.sin(hx * 0.002) * 30 + Math.sin(hx * 0.005) * 15)
                ctx.lineTo(totalContentWidth, root.height)
                ctx.lineTo(0, root.height)
                ctx.closePath()
                ctx.fill()

                ctx.globalAlpha = 0.04
                ctx.beginPath()
                ctx.moveTo(0, root.height * 0.6)
                for (var hx2 = 0; hx2 <= totalContentWidth; hx2 += 8)
                    ctx.lineTo(hx2, root.height * 0.6 + Math.sin(hx2 * 0.003 + 1) * 40 + Math.cos(hx2 * 0.001) * 25)
                ctx.lineTo(totalContentWidth, root.height)
                ctx.lineTo(0, root.height)
                ctx.closePath()
                ctx.fill()
                ctx.globalAlpha = 1.0

                // Ground band
                ctx.globalAlpha = 0.05
                ctx.fillStyle = "#5A9336"
                ctx.fillRect(0, root.height * 0.82, totalContentWidth, root.height * 0.18)
                ctx.globalAlpha = 1.0

                // Simple trees on hills
                ctx.globalAlpha = 0.07
                var treeCount = Math.floor(totalContentWidth / 280)
                for (var ti = 0; ti < treeCount; ti++) {
                    var tx = 150 + ti * 280
                    var ty = root.height * 0.72 + Math.sin(tx * 0.003) * 20
                    ctx.fillStyle = "#8B5E3C"
                    ctx.fillRect(tx - 2, ty, 4, 16)
                    ctx.fillStyle = "#5C6B28"
                    ctx.beginPath()
                    ctx.arc(tx, ty - 4, 10, 0, Math.PI * 2)
                    ctx.fill()
                }
                ctx.globalAlpha = 1.0

                // ---- Track line ----
                var lineY = trackY
                ctx.strokeStyle = "#8A7A60"
                ctx.lineWidth = 5
                ctx.lineCap = "round"
                ctx.beginPath()
                ctx.moveTo(trackStartX - 40, lineY)
                ctx.lineTo(trackStartX + (listModel.count - 1) * nodeSpacing + 40, lineY)
                ctx.stroke()

                // Dashed center line
                ctx.strokeStyle = "rgba(255,255,255,0.12)"
                ctx.lineWidth = 1.5
                var dashOn = true
                var dashCnt = 0
                ctx.beginPath()
                for (var dx = trackStartX - 40; dx <= trackStartX + (listModel.count - 1) * nodeSpacing + 40; dx += 4) {
                    if (dashOn) {
                        if (dashCnt === 0) ctx.moveTo(dx, lineY)
                        else ctx.lineTo(dx, lineY)
                    }
                    dashCnt++
                    if (dashOn && dashCnt >= 8) { ctx.stroke(); ctx.beginPath(); dashOn = false; dashCnt = 0 }
                    else if (!dashOn && dashCnt >= 5) { dashOn = true; dashCnt = 0 }
                }
                ctx.stroke()
            }
            Component.onCompleted: requestPaint()
        }

        // ---- Level nodes ----
        Repeater {
            id: nodeRepeater
            model: listModel

            Item {
                id: nodeContainer
                x: model.nodeX - 48
                y: trackY - 48
                width: 96
                height: 96

                property bool isCompleted: model.stars > 0
                property bool isUnlocked: !isCompleted && model.unlocked
                property bool isLocked: !isCompleted && !isUnlocked
                property int nodeStars: model.stars

                // Node circle + icon canvas
                Canvas {
                    id: nodeCanvas
                    width: 96
                    height: 96
                    property bool completed: nodeContainer.isCompleted

                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        var cx = 48, cy = 48

                        // Circle background
                        ctx.beginPath()
                        ctx.arc(cx, cy, nodeR, 0, Math.PI * 2)
                        if (completed) {
                            ctx.fillStyle = Theme.btnGreen
                            ctx.fill()
                            ctx.strokeStyle = Theme.btnGreenHover
                            ctx.lineWidth = 2
                            ctx.stroke()
                        } else if (nodeContainer.isUnlocked) {
                            ctx.fillStyle = "#3A4A5C"
                            ctx.fill()
                            ctx.strokeStyle = "#5C6B28"
                            ctx.lineWidth = 2
                            ctx.stroke()
                        } else {
                            ctx.fillStyle = "#2A3040"
                            ctx.fill()
                            ctx.strokeStyle = "rgba(255,255,255,0.2)"
                            ctx.lineWidth = 1
                            ctx.stroke()
                        }

                        // Icon or symbol inside circle
                        if (completed || nodeContainer.isUnlocked) {
                            drawNodeIcon(ctx, index, cx, cy)
                            // Small checkmark badge for completed
                            if (completed) {
                                var bx = cx + 22, by = cy + 22
                                ctx.fillStyle = Theme.btnGreen
                                ctx.beginPath(); ctx.arc(bx, by, 10, 0, Math.PI*2); ctx.fill()
                                ctx.strokeStyle = Theme.btnGreenHover; ctx.lineWidth = 1.5
                                ctx.beginPath(); ctx.arc(bx, by, 10, 0, Math.PI*2); ctx.stroke()
                                ctx.strokeStyle = "#FFFFFF"; ctx.lineWidth = 2; ctx.lineCap = "round"; ctx.lineJoin = "round"
                                ctx.beginPath()
                                ctx.moveTo(bx - 4, by + 1)
                                ctx.lineTo(bx - 1, by + 4)
                                ctx.lineTo(bx + 5, by - 3)
                                ctx.stroke()
                            }
                        } else {
                            // Lock icon
                            ctx.fillStyle = "rgba(255,255,255,0.25)"
                            ctx.beginPath()
                            ctx.roundedRect(cx - 11, cy - 5, 22, 17, 3, 3)
                            ctx.fill()
                            ctx.beginPath()
                            ctx.arc(cx, cy - 7, 8, Math.PI, 0)
                            ctx.strokeStyle = "rgba(255,255,255,0.25)"
                            ctx.lineWidth = 3
                            ctx.stroke()
                        }

                        // Golden ring for completed
                        if (completed) {
                            ctx.strokeStyle = "#F5C518"
                            ctx.lineWidth = 2
                            ctx.globalAlpha = 0.6
                            ctx.beginPath()
                            ctx.arc(cx, cy, nodeR + 4, 0, Math.PI * 2)
                            ctx.stroke()
                            ctx.globalAlpha = 1.0
                        }
                    }
                    Component.onCompleted: requestPaint()
                }

                // Dark overlay for locked
                Rectangle {
                    x: nodeCanvas.x; y: nodeCanvas.y
                    width: nodeCanvas.width; height: nodeCanvas.height
                    radius: nodeR
                    color: Qt.rgba(0.1, 0.12, 0.16, 0.55)
                    visible: nodeContainer.isLocked
                }

                // Level name below node
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    y: parent.height + 6
                    text: model.name
                    color: nodeContainer.isLocked ? "#5A6070" : Theme.textLight
                    font.family: Theme.fontUI
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    width: 110
                    elide: Text.ElideRight
                }

                // Stars below name
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    y: parent.height + 22
                    spacing: 1
                    visible: !nodeContainer.isLocked
                    Repeater {
                        model: 3
                        Text {
                            text: "★"
                            color: index < nodeContainer.nodeStars ? Theme.starGold : Theme.textMuted
                            font.pixelSize: 10
                        }
                    }
                }

                // Pulsing glow for unlocked
                Rectangle {
                    visible: nodeContainer.isUnlocked
                    x: -8; y: -8
                    width: 112; height: 112
                    radius: 56
                    color: "transparent"
                    border.width: 2.5
                    property real gOp: 0.5
                    border.color: Qt.rgba(0.36, 0.42, 0.16, gOp)
                    SequentialAnimation on gOp {
                        loops: Animation.Infinite
                        NumberAnimation { from: 0.5; to: 0.1; duration: 1500; easing.type: Easing.InOutSine }
                        NumberAnimation { from: 0.1; to: 0.5; duration: 1500; easing.type: Easing.InOutSine }
                    }
                    NumberAnimation on scale {
                        loops: Animation.Infinite
                        from: 1.0; to: 1.35; duration: 1500; easing.type: Easing.InOutSine
                    }
                }

                // Tooltip on hover
                Rectangle {
                    id: tip
                    width: tipCol.width + 20
                    height: tipCol.height + 14
                    x: (parent.width - width) / 2
                    y: -height - 10
                    radius: 7
                    color: Theme.panelBg
                    border.width: 1
                    border.color: Theme.borderDim
                    opacity: nMa.containsMouse && !nodeContainer.isLocked ? 1.0 : 0.0
                    visible: opacity > 0
                    z: 100
                    Behavior on opacity { NumberAnimation { duration: 120 } }

                    Column {
                        id: tipCol
                        anchors.centerIn: parent
                        spacing: 2
                        Text {
                            text: model.name; color: Theme.textLight
                            font.family: Theme.fontUI; font.pixelSize: 12; font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: model.gridW + "×" + model.gridH + " 地图"; color: Theme.textDim
                            font.family: Theme.fontCode; font.pixelSize: 9
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Row {
                            spacing: 2; anchors.horizontalCenter: parent.horizontalCenter
                            Repeater {
                                model: 3
                                Text { text: "★"; color: index < nodeContainer.nodeStars ? Theme.starGold : Theme.textMuted; font.pixelSize: 10 }
                            }
                        }
                        Text {
                            text: model.bestTime > 0 ? ("最佳: " + model.bestTime + "s") : ""
                            color: Theme.textMuted; font.family: Theme.fontCode; font.pixelSize: 8
                            visible: model.bestTime > 0; anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }

                // Mouse area
                MouseArea {
                    id: nMa
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: nodeContainer.isLocked ? Qt.ArrowCursor : Qt.PointingHandCursor
                    enabled: !nodeContainer.isLocked
                    onPressed: { clickAnim.from = 0.9; clickAnim.start() }
                    onReleased: {
                        clickAnimStop.start()
                        appVm.openLevel(model.id)
                        navigator.push(gameViewPage)
                    }
                }

                // Click animation
                NumberAnimation { id: clickAnim; target: nodeContainer; property: "scale"; to: 1.0; duration: 80; easing.type: Easing.OutQuad }
                NumberAnimation { id: clickAnimStop; target: nodeContainer; property: "scale"; from: 0.9; to: 1.08; duration: 120; easing.type: Easing.OutBack }

                scale: nMa.pressed ? 0.9 : (nMa.containsMouse && !nodeContainer.isLocked ? 1.08 : 1.0)
                Behavior on scale { NumberAnimation { duration: 100; easing.type: Easing.OutQuad } }

                opacity: nodeContainer.isLocked ? 0.35 : 1.0
            }
        }
    }

    // =====================================================================
    // UI overlay — back button
    // =====================================================================
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 12
        anchors.leftMargin: 16
        z: 100
        width: 32; height: 32; radius: 6
        color: backMa.containsMouse ? Theme.btnGreenHover : Theme.btnGreen
        Text { anchors.centerIn: parent; text: "←"; color: "white"; font.pixelSize: 16; font.weight: Font.Bold }
        MouseArea { id: backMa; anchors.fill: parent; hoverEnabled: true; onClicked: navigator.pop() }
    }
}
