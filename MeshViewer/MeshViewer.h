#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MeshViewer.h"

class MeshViewer : public QMainWindow
{
    Q_OBJECT

public:
    MeshViewer(QWidget *parent = Q_NULLPTR);

private:
    Ui::MeshViewerClass ui;
};
