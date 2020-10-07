#ifndef DOTPLOT_H
#define DOTPLOT_H

#include "displayinterface.h"
#include "dotplotcontrols.h"
#include "dotplotwidget.h"

class DotPlot : public QObject, DisplayInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "hobbits.DisplayInterface.DotPlot")
    Q_INTERFACES(DisplayInterface)

public:
    DotPlot();

    DisplayInterface* createDefaultDisplay() override;

    QString name() override;
    QString description() override;
    QStringList tags() override;

    QWidget* display(QSharedPointer<DisplayHandle> displayHandle) override;
    QWidget* controls(QSharedPointer<DisplayHandle> displayHandle) override;

private:
    void initialize(QSharedPointer<DisplayHandle> displayHandle);
    DotPlotWidget* m_displayWidget;
    DotPlotControls* m_controlsWidget;
};

#endif // DOTPLOT_H
