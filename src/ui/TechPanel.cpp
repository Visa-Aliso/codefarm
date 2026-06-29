#include "TechPanel.h"
#include "core/GameState.h"
#include "core/TechTree.h"
#include <QVBoxLayout>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsPathItem>
#include <QPushButton>
#include <QMessageBox>
#include <QPainter>
#include <QFont>
#include <QMap>
#include <QTimer>
#include <QLabel>
#include <QHBoxLayout>

TechPanel::TechPanel(GameState *state, QWidget *parent)
    : QWidget(parent)
    , m_gameState(state)
    , m_view(nullptr)
    , m_scene(nullptr)
    , m_detailTitle(nullptr)
    , m_detailBody(nullptr)
    , m_upgradeButton(nullptr)
{
    setupUI();
}

void TechPanel::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    auto *title = new QLabel("科技进化路线", this);
    title->setStyleSheet("color: #40503A; font-size: 22px; font-weight: 900; background: transparent;");
    mainLayout->addWidget(title);

    auto *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(12);
    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene, this);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    m_view->setStyleSheet("QGraphicsView { background: #F6F0DF; border: 1px solid #DAC9A7; border-radius: 18px; }");
    contentLayout->addWidget(m_view, 1);

    auto *detailPanel = new QWidget(this);
    detailPanel->setFixedWidth(280);
    detailPanel->setStyleSheet("QWidget { background: #FFF9EB; border: 1px solid #DAC9A7; border-radius: 18px; }");
    auto *detailLayout = new QVBoxLayout(detailPanel);
    detailLayout->setContentsMargins(18, 16, 18, 16);
    detailLayout->setSpacing(12);
    m_detailTitle = new QLabel("选择一个科技", detailPanel);
    m_detailTitle->setWordWrap(true);
    m_detailTitle->setStyleSheet("color: #40503A; font-size: 18px; font-weight: 900; background: transparent;");
    m_detailBody = new QLabel("点击左侧节点查看效果、前置条件和升级费用。", detailPanel);
    m_detailBody->setWordWrap(true);
    m_detailBody->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_detailBody->setStyleSheet("color: #6D6654; font-size: 13px; line-height: 150%; background: transparent;");
    m_upgradeButton = new QPushButton("升级", detailPanel);
    m_upgradeButton->setEnabled(false);
    m_upgradeButton->setStyleSheet(R"(
        QPushButton { background: #C6DFAE; color: #3F4B38; border: none; border-radius: 12px; padding: 10px 16px; font-weight: 900; }
        QPushButton:hover { background: #D8EBC5; }
        QPushButton:disabled { background: #DDD6C8; color: #9A927F; }
    )");
    detailLayout->addWidget(m_detailTitle);
    detailLayout->addWidget(m_detailBody, 1);
    detailLayout->addWidget(m_upgradeButton);
    contentLayout->addWidget(detailPanel);
    mainLayout->addLayout(contentLayout, 1);

    connect(m_upgradeButton, &QPushButton::clicked, this, [this]() {
        const TechNode *node = nodeById(m_selectedNodeId);
        if (!node) return;
        if (!m_gameState->techTree()->canUnlock(node->id)) return;
        if (!m_gameState->spendGold(node->cost)) {
            QMessageBox::information(this, tr("金币不足"), tr("需要 %1 金币").arg(node->cost));
            return;
        }
        m_gameState->techTree()->unlock(node->id);
        m_gameState->applyTechNode(node->id);
        QTimer::singleShot(0, this, [this]() { rebuild(); selectNode(m_selectedNodeId); });
    });
}

void TechPanel::rebuild()
{
    m_scene->clear();

    const auto &nodes = m_gameState->techTree()->allNodes();
    QMap<QString, QPointF> pos;
    QMap<QString, TechNode> byId;
    QVector<int> branchCount(4, 0);
    const QString branchNames[] = {"农业科技", "机械科技", "编程科技", "土地科技"};
    const QColor branchColors[] = {QColor("#BCD9A4"), QColor("#B8D7DF"), QColor("#D9CF9F"), QColor("#D5BEA4")};

    for (const auto &node : nodes) {
        const int b = static_cast<int>(node.branch);
        const int i = branchCount[b]++;
        pos[node.id] = QPointF(170 + i * 178, 90 + b * 150);
        byId[node.id] = node;
    }

    const QMap<QString, QPointF> handPlaced = {
        {"A0", {140, 160}}, {"B0", {140, 320}}, {"C0", {140, 480}}, {"D0", {140, 640}},
        {"D1", {360, 535}}, {"C1", {540, 430}}, {"C2", {735, 360}}, {"A1", {735, 190}},
        {"D2", {940, 535}}, {"C3", {960, 265}}, {"D4", {1110, 145}}, {"B1", {1130, 430}},
        {"C4", {1240, 610}}, {"A2", {1320, 225}}, {"A3", {1380, 380}},
        {"C5", {1500, 535}}, {"D3", {1500, 710}}, {"B2", {1600, 410}},
        {"C6", {1740, 650}}, {"A4", {1780, 220}}, {"A5", {1820, 390}}, {"D5", {1900, 805}},
        {"B3", {2020, 570}}, {"C7", {2070, 340}}, {"D6", {2140, 760}},
        {"B4", {2300, 510}}, {"A6", {2320, 270}}, {"C8", {2520, 470}},
        {"D7", {2500, 720}}, {"D8", {2720, 720}}, {"D9", {2940, 720}},
    };
    for (auto it = handPlaced.begin(); it != handPlaced.end(); ++it)
        if (pos.contains(it.key()))
            pos[it.key()] = it.value();

    auto *root = m_scene->addEllipse(18, 285, 90, 90, QPen(QColor("#8CA77A"), 2), QBrush(QColor("#E5F0D9")));
    root->setToolTip("Code Farm 智能果园：所有科技从这里向四个方向展开");
    auto *rootText = m_scene->addText("起点");
    rootText->setDefaultTextColor(QColor("#41503A"));
    rootText->setFont(QFont("LXGW WenKai", 13, QFont::Bold));
    rootText->setPos(45, 315);

    for (int b = 0; b < 4; ++b) {
        auto *label = m_scene->addText(branchNames[b]);
        label->setDefaultTextColor(QColor("#526048"));
        label->setFont(QFont("LXGW WenKai", 15, QFont::Bold));
        label->setPos(130, 48 + b * 150);
        m_scene->addLine(108, 330, 170, 120 + b * 150, QPen(branchColors[b].darker(120), 2));
    }

    for (const auto &node : nodes) {
        const QPointF p = pos[node.id];
        for (const auto &pre : node.prerequisites) {
            if (!pos.contains(pre)) continue;
            const QPointF a = pos[pre] + QPointF(132, 35);
            const QPointF b = p + QPointF(0, 35);
            QPainterPath path(a);
            const qreal midX = (a.x() + b.x()) / 2.0;
            path.cubicTo(QPointF(midX, a.y()), QPointF(midX, b.y()), b);
            m_scene->addPath(path, QPen(QColor("#B6AA8F"), 2));
        }
    }

    for (auto &node : nodes) {
        const int b = static_cast<int>(node.branch);
        const QPointF p = pos[node.id];
        bool unlocked = m_gameState->techTree()->isUnlocked(node.id);
        bool canBuy = m_gameState->techTree()->canUnlock(node.id);

        const QColor bg = unlocked ? branchColors[b].lighter(115) : (canBuy ? QColor("#FFF1BD") : QColor("#DDD7C9"));
        QPainterPath cardPath;
        cardPath.addRoundedRect(QRectF(p, QSizeF(132, 70)), 16, 16);
        auto *card = m_scene->addPath(cardPath,
                                      QPen(node.id == m_selectedNodeId ? QColor("#E4B95E") : (unlocked ? branchColors[b].darker(130) : QColor("#B9AC91")), node.id == m_selectedNodeId ? 3.0 : 1.5),
                                      QBrush(bg));
        card->setToolTip(node.effect);
        card->setData(0, node.id);
        card->setFlag(QGraphicsItem::ItemIsSelectable, true);

        auto *name = m_scene->addText(node.name);
        name->setDefaultTextColor(unlocked ? QColor("#38543A") : QColor("#6B604D"));
        name->setFont(QFont("LXGW WenKai", 10, QFont::Bold));
        name->setTextWidth(118);
        name->setPos(p + QPointF(9, 7));

        auto *cost = m_scene->addText(unlocked ? "已解锁" : QString("%1 金币").arg(node.cost));
        cost->setDefaultTextColor(QColor("#756A55"));
        cost->setFont(QFont("LXGW WenKai", 8));
        cost->setPos(p + QPointF(9, 39));

        if (canBuy && !unlocked) {
            auto *hint = m_scene->addText("可升级");
            hint->setDefaultTextColor(QColor("#8A6B2E"));
            hint->setFont(QFont("LXGW WenKai", 8, QFont::Bold));
            hint->setPos(p + QPointF(82, 43));
        }
    }
    m_scene->setSceneRect(m_scene->itemsBoundingRect().adjusted(-30, -30, 80, 60));
    connect(m_scene, &QGraphicsScene::selectionChanged, this, [this]() {
        const auto items = m_scene->selectedItems();
        if (items.isEmpty()) return;
        const QString nodeId = items.first()->data(0).toString();
        if (!nodeId.isEmpty())
            selectNode(nodeId);
    });
    refreshDetails();
}

const TechNode *TechPanel::nodeById(const QString &id) const
{
    for (const auto &node : m_gameState->techTree()->allNodes())
        if (node.id == id)
            return &node;
    return nullptr;
}

void TechPanel::selectNode(const QString &id)
{
    m_selectedNodeId = id;
    refreshDetails();
}

void TechPanel::refreshDetails()
{
    const TechNode *node = nodeById(m_selectedNodeId);
    if (!node) {
        m_detailTitle->setText("选择一个科技");
        m_detailBody->setText("点击左侧节点查看效果、前置条件和升级费用。");
        m_upgradeButton->setEnabled(false);
        return;
    }

    QStringList missing;
    for (const auto &pre : node->prerequisites)
        if (!m_gameState->techTree()->isUnlocked(pre))
            if (const TechNode *preNode = nodeById(pre)) missing << preNode->name;

    const bool unlocked = m_gameState->techTree()->isUnlocked(node->id);
    const bool canBuy = m_gameState->techTree()->canUnlock(node->id);
    m_detailTitle->setText(node->name);
    m_detailBody->setText(QString("效果：%1\n\n费用：%2 金币\n状态：%3%4")
                              .arg(node->effect)
                              .arg(node->cost)
                              .arg(unlocked ? "已解锁" : (canBuy ? "可升级" : "前置不足"))
                              .arg(missing.isEmpty() ? "" : QString("\n缺少：%1").arg(missing.join("、"))));
    m_upgradeButton->setEnabled(canBuy && !unlocked);
}
