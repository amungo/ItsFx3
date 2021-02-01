#ifndef DIALOGCONFIG_H
#define DIALOGCONFIG_H

#include <QDialog>
#include <map>
#include "hwfx3/fx3config.h"

namespace Ui {
class DialogConfig;
}

class DialogConfig : public QDialog
{
    Q_OBJECT

public:
    explicit DialogConfig(FX3Config *cfg, QWidget *parent = 0);
    ~DialogConfig();

private:
    Ui::DialogConfig *ui;
    FX3Config* cfg;
    std::map<DriverType_t, int> drivers_combobox_map;
private slots:
    void boardChanged(int);
    void reSetFields();
    void openFileImg(bool);
    void openFileAddFw(bool);
    void openFileBit(bool);
    void changedSubs(int);
    void setConfigValues();
};

#endif // DIALOGCONFIG_H
