#include "GameEngine.h"
#include <QRegularExpression>
#include <algorithm>
#include <cmath>

GameEngine::GameEngine(QObject *parent) : QObject(parent)
{
    m_rng.seed(std::random_device{}());
    m_tickTimer.setInterval(500);
    connect(&m_tickTimer, &QTimer::timeout, this, &GameEngine::tick);
}

void GameEngine::loadLevel(const Level &level)
{
    m_level = level;
    m_cells = QVector<Cell>(level.size.width() * level.size.height());
    for (auto &cell : m_cells) {
        if (level.setup == "tilled") {
            cell.tilled = true;
            cell.water = 0.45;
        } else if (level.setup == "dry") {
            cell.water = 0;
        }
    }
    m_goals = level.goals;
    m_drone = {0, 0};
    m_pc = 0;
    m_elapsed = 0;
    m_commands.clear();
    m_running = false;
    m_tickTimer.stop();
    emit stateChanged();
}

void GameEngine::run(const QString &code)
{
    m_commands = parseCommands(code);
    if (m_commands.isEmpty()) {
        emit logMessage("没有可执行命令。");
        return;
    }
    m_running = true;
    m_tickTimer.start();
    emit stateChanged();
}

void GameEngine::step(const QString &code)
{
    if (m_commands.isEmpty()) m_commands = parseCommands(code);
    executeNext();
}

void GameEngine::stop()
{
    m_running = false;
    m_tickTimer.stop();
    emit stateChanged();
}

QString GameEngine::goalText() const
{
    QString text;
    for (const auto &g : m_goals) {
        const CropInfo *info = cropInfo(g.crop);
        text += QString("%1  %2    %3 / %4\n").arg(info ? info->icon : g.crop).arg(info ? info->name : g.crop).arg(g.done).arg(g.target);
    }
    return text.trimmed();
}

void GameEngine::tick()
{
    if (m_running) executeNext();
}

Cell &GameEngine::current()
{
    return m_cells[m_drone.y() * m_level.size.width() + m_drone.x()];
}

void GameEngine::executeNext()
{
    if (m_commands.isEmpty()) return;
    if (m_elapsed >= m_level.seconds) {
        stop();
        emit levelFinished(false, 0);
        return;
    }
    const QString line = m_commands.at(m_pc % m_commands.size());
    emit commandHighlight(m_pc % m_commands.size(), line);
    ++m_pc;
    runCommand(line);
    growCrops();
    m_elapsed += 0.5;
    if (complete()) {
        stop();
        const double ratio = (m_level.seconds - m_elapsed) / std::max(1, m_level.seconds);
        int award = ratio > 0.55 ? 3 : (ratio > 0.25 ? 2 : 1);
        emit levelFinished(true, award);
        return;
    }
    if (m_elapsed >= m_level.seconds && !complete()) {
        stop();
        emit levelFinished(false, 0);
        return;
    }
    emit stateChanged();
}

void GameEngine::runCommand(const QString &line)
{
    static const QRegularExpression rx("^([A-Za-z_][A-Za-z0-9_]*)\\s*\\((.*)\\)\\s*$");
    const auto m = rx.match(line);
    if (!m.hasMatch()) {
        emit logMessage(QString("跳过无法解析的语句：%1").arg(line));
        return;
    }
    const QString fn = m.captured(1);
    QString arg = m.captured(2).trimmed();
    arg.remove('"');
    arg.remove('\'');
    if (!m_level.functions.contains(fn)) {
        emit logMessage(QString("%1 本关尚未解锁。").arg(fn));
        return;
    }
    if (fn == "move") doMove(arg);
    else if (fn == "till") doTill();
    else if (fn == "plant") doPlant(arg);
    else if (fn == "water") doWater();
    else if (fn == "fertilize") doFertilize();
    else if (fn == "debug") doDebug();
    else if (fn == "harvest") doHarvest();
    else if (fn == "get_pos") emit logMessage(QString("位置：(%1, %2)").arg(m_drone.x()).arg(m_drone.y()));
    else if (fn == "get_current") describeCurrent();
    else if (fn == "get_goals") emit logMessage(goalText().replace('\n', "  "));
    else emit logMessage(QString("未知命令：%1").arg(fn));
}

void GameEngine::doMove(const QString &dir)
{
    QPoint next = m_drone;
    if (dir == "right") next.rx()++;
    else if (dir == "left") next.rx()--;
    else if (dir == "down") next.ry()++;
    else if (dir == "up") next.ry()--;
    else {
        emit logMessage(QString("move 参数无效：%1").arg(dir));
        return;
    }
    if (next.x() < 0 || next.y() < 0 || next.x() >= m_level.size.width() || next.y() >= m_level.size.height()) {
        emit logMessage("无人机碰到边界。");
        return;
    }
    m_drone = next;
    emit logMessage(QString("移动到 (%1, %2)。").arg(m_drone.x()).arg(m_drone.y()));
}

void GameEngine::doTill()
{
    auto &cell = current();
    if (!cell.crop.isEmpty()) {
        emit logMessage("已有作物，不能开垦。");
        return;
    }
    cell.tilled = true;
    emit logMessage("开垦完成。");
}

void GameEngine::doPlant(const QString &crop)
{
    auto &cell = current();
    if (!cell.tilled) {
        emit logMessage("土地未开垦。");
        return;
    }
    if (!cell.crop.isEmpty()) {
        emit logMessage("当前地块已有作物。");
        return;
    }
    const bool needed = std::any_of(m_goals.begin(), m_goals.end(), [&](const Goal &g) { return g.crop == crop; });
    if (!cropInfo(crop) || !needed) {
        emit logMessage(QString("本关不需要种植 %1。").arg(crop));
        return;
    }
    cell.crop = crop;
    cell.progress = 0.05;
    cell.water = std::max(cell.water, 0.25);
    emit logMessage(QString("种下 %1。").arg(cropInfo(crop)->name));
}

void GameEngine::doWater()
{
    auto &cell = current();
    cell.water = std::min(1.0, cell.water + 0.45);
    emit logMessage("浇水完成。");
}

void GameEngine::doFertilize()
{
    auto &cell = current();
    if (cell.crop.isEmpty()) {
        emit logMessage("没有作物可施肥。");
        return;
    }
    cell.fertilizer = 10;
    emit logMessage("施肥完成，成长加速。");
}

void GameEngine::doDebug()
{
    auto &cell = current();
    if (!cell.bug) {
        emit logMessage("当前地块没有 Bug。");
        return;
    }
    cell.bug = false;
    emit logMessage("Bug 已修复。");
}

void GameEngine::doHarvest()
{
    auto &cell = current();
    if (cell.crop.isEmpty()) {
        emit logMessage("没有作物可收割。");
        return;
    }
    if (cell.bug) {
        emit logMessage("作物存在 Bug，先 debug()。");
        return;
    }
    if (cell.progress < 1.0) {
        emit logMessage("作物还未成熟。");
        return;
    }
    for (auto &goal : m_goals) {
        if (goal.crop == cell.crop && goal.done < goal.target) {
            ++goal.done;
            emit logMessage(QString("收获 %1，进度 %2/%3。").arg(cropInfo(cell.crop)->name).arg(goal.done).arg(goal.target));
            break;
        }
    }
    cell = Cell{true, "", 0, std::max(0.2, cell.water * 0.5), 0, false};
}

void GameEngine::describeCurrent()
{
    const auto &cell = current();
    QString name = cell.crop.isEmpty() ? "无" : cropInfo(cell.crop)->name;
    emit logMessage(QString("地块：开垦=%1 作物=%2 成长=%3% 水分=%4% Bug=%5")
                    .arg(cell.tilled ? "是" : "否")
                    .arg(name)
                    .arg(int(cell.progress * 100))
                    .arg(int(cell.water * 100))
                    .arg(cell.bug ? "是" : "否"));
}

void GameEngine::growCrops()
{
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    const double bugBoost = (m_level.setup == "bugs") ? 1.9 : 1.0;
    for (auto &cell : m_cells) {
        if (cell.crop.isEmpty() || cell.bug) continue;
        const CropInfo *info = cropInfo(cell.crop);
        if (!info) continue;
        const bool enoughWater = cell.water >= info->minWater;
        double speed = enoughWater ? 1.0 / info->ticks : 0.25 / info->ticks;
        if (cell.fertilizer > 0) {
            speed *= 2.0;
            --cell.fertilizer;
        }
        cell.progress = std::min(1.0, cell.progress + speed);
        cell.water = std::max(0.0, cell.water - 0.025);
        if (cell.progress < 1.0 && dist(m_rng) < info->bugRate * bugBoost) {
            cell.bug = true;
        }
    }
}

bool GameEngine::complete() const
{
    return std::all_of(m_goals.begin(), m_goals.end(), [](const Goal &g) { return g.done >= g.target; });
}

QStringList GameEngine::parseCommands(const QString &code) const
{
    QStringList result;
    for (QString line : code.split('\n')) {
        line = line.trimmed();
        const int commentAt = line.indexOf('#');
        if (commentAt >= 0) line = line.left(commentAt).trimmed();
        if (!line.isEmpty()) result << line;
    }
    return result;
}
