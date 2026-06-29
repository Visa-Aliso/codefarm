#ifndef CLOUDANIMATION_H
#define CLOUDANIMATION_H

#include <QObject>

// Cloud animation is implemented inline in MenuScreen for Phase 1.
// This file provides a standalone wrapper for later use.

class CloudAnimation : public QObject {
    Q_OBJECT
public:
    explicit CloudAnimation(QObject *parent = nullptr) : QObject(parent) {}
};

#endif // CLOUDANIMATION_H
