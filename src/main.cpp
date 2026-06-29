#include <QApplication>
#include <QFontDatabase>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Code Farm");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("CodeFarm");

    // Global stylesheet — low saturation, rounded, card-style
    app.setStyleSheet(R"(
        * {
            font-family: "LXGW WenKai", "ZCOOL KuaiLe", "Noto Sans CJK SC", "Microsoft YaHei UI", sans-serif;
        }
        QMainWindow { background-color: #CFE6EA; }
        QDockWidget {
            background: #FFF8EA;
            border: 1px solid #D4C9B5;
            border-radius: 12px;
            font-size: 14px;
            color: #4A3F35;
        }
        QDockWidget::title {
            background: #E8DFD0;
            border-radius: 10px;
            padding: 6px 12px;
            color: #4A3F35;
            font-weight: bold;
        }
        QPushButton {
            background-color: #BFD8A7;
            color: #3F4B38;
            border: none;
            border-radius: 12px;
            padding: 10px 24px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #D4E7C4;
        }
        QPushButton:pressed {
            background-color: #8FBC8F;
        }
        QScrollArea {
            background: transparent;
            border: none;
        }
        QScrollBar:vertical {
            background: #F0EDE5;
            width: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #C4B8A5;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QTabWidget::pane {
            background: #FAF7F0;
            border: 1px solid #D4C9B5;
            border-radius: 8px;
        }
        QTabBar::tab {
            background: #E8DFD0;
            border-radius: 8px;
            padding: 6px 16px;
            margin: 2px;
            color: #4A3F35;
        }
        QTabBar::tab:selected {
            background: #D4C9B5;
            font-weight: bold;
        }
        QToolTip {
            background: #FAF7F0;
            color: #4A3F35;
            border: 1px solid #D4C9B5;
            border-radius: 8px;
            padding: 6px 10px;
            font-size: 13px;
        }
    )");

    MainWindow window;
    window.show();

    return app.exec();
}
