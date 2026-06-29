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

    // Farm tiles
    readonly property color tileEmpty: "#C8B078"
    readonly property color tileEmptySide: "#A89058"
    readonly property color tileTilled: "#8B5E3C"
    readonly property color tileTilledSide: "#6B4020"
    readonly property color tilePlanted: "#7AB356"
    readonly property color tilePlantedSide: "#5A9336"
    readonly property color tileMature: "#4A8C3A"
    readonly property color tileMatureSide: "#2A6C1A"
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
