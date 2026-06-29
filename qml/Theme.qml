pragma Singleton
import QtQuick

QtObject {
    // Backgrounds (unified across all pages)
    readonly property color bgMain: "#7A9EAF"
    readonly property color skyBlue: "#7A9EAF"
    readonly property color menuDark: "#7A9EAF"
    readonly property color editorBg: "#1B1F23"
    readonly property color editorTitleBar: "#252A30"
    readonly property color editorStatusBar: "#1F2428"

    // Buttons - olive green square style (from reference)
    readonly property color btnGreen: "#5C6B28"
    readonly property color btnGreenHover: "#6E7E33"
    readonly property color btnGreenPress: "#4A5720"
    readonly property color btnSecondary: "#4A5568"
    readonly property color btnSecondaryHover: "#5A6578"

    // Farm block colors (top-down 3D block style)
    readonly property color blockGrassTop: "#7DA84E"
    readonly property color blockGrassFront: "#6B8C3A"
    readonly property color blockGrassSide: "#5A7A30"
    readonly property color blockSoilTop: "#A07848"
    readonly property color blockSoilFront: "#8B6538"
    readonly property color blockSoilSide: "#6B4528"
    readonly property color blockWetSoilTop: "#7A5830"
    readonly property color blockRockTop: "#8A8A8A"
    readonly property color blockRockFront: "#6A6A6A"
    readonly property color blockRockSide: "#5A5A5A"
    readonly property color blockShadow: "#20000000"

    // Hedge / border bushes
    readonly property color hedgeBody: "#4A7A25"
    readonly property color hedgeLight: "#6AA840"
    readonly property color hedgeDark: "#3A6218"
    readonly property color hedgeTrunk: "#8B6B42"

    // Drone
    readonly property color droneBody: "#E8862A"
    readonly property color droneHighlight: "#F0A040"
    readonly property color droneArm: "#5A5A5A"
    readonly property color droneRotor: "#505050"
    readonly property color droneLed: "#4CAF50"

    // Legacy tile colors (kept for MiniMap compatibility)
    readonly property color tileEmpty: "#C8B078"
    readonly property color tileTilled: "#8B5E3C"
    readonly property color tilePlanted: "#7AB356"
    readonly property color tileMature: "#4A8C3A"
    readonly property color tileWater: "#5AAFCF"
    readonly property color tileBug: "#CF5A5A"
    readonly property color tileFertilized: "#D4A840"

    // Text
    readonly property color textLight: "#FFFFFF"
    readonly property color textDim: "#A0B0C0"
    readonly property color textMuted: "#6B7B8B"
    readonly property color textDark: "#2C3E2D"

    // UI
    readonly property color overlayDark: "#80000000"
    readonly property color barBg: "#4D000000"
    readonly property color panelBg: "#E61E2228"
    readonly property color borderDim: "#40FFFFFF"

    // Status
    readonly property color statusRunning: "#4CAF50"
    readonly property color statusPaused: "#FFC107"
    readonly property color statusError: "#E07848"
    readonly property color starGold: "#F5C518"

    // Fonts
    readonly property string fontUI: "Nunito, Segoe UI, Arial"
    readonly property string fontCode: "JetBrains Mono, Consolas, monospace"
}
