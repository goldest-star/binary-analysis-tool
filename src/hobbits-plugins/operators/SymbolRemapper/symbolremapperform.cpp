#include "symbolremapperform.h"
#include "ui_symbolremapperform.h"
#include <QJsonArray>
#include <QtMath>

SymbolRemapperForm::SymbolRemapperForm(QSharedPointer<ParameterDelegate> delegate) :
    ui(new Ui::SymbolRemapperForm()),
    m_delegate(delegate),
    m_paramHelper(new ParameterHelper(delegate)),
    m_remapModel(new RemapModel())
{
    ui->setupUi(this);
    m_remapModel->setRemapLength(ui->sb_bitLength->value());
    ui->tv_mappings->setModel(m_remapModel);
    connect(ui->sb_bitLength, SIGNAL(valueChanged(int)), m_remapModel, SLOT(setRemapLength(int)));
}

SymbolRemapperForm::~SymbolRemapperForm()
{
    delete ui;
    delete m_remapModel;
}

QString SymbolRemapperForm::title()
{
    return "Configure Symbol Remapping";
}

bool SymbolRemapperForm::setParameters(QJsonObject parameters)
{
    if (!m_delegate->validate((parameters))) {
        return false;
    }

    QJsonArray mappingsArray = parameters.value("mappings").toArray();
    ui->sb_bitLength->setValue(int(log2(mappingsArray.size())));
    int row = 0;
    for (auto valueRef : mappingsArray) {
        QJsonObject mappingValue = valueRef.toObject();
        m_remapModel->setData(m_remapModel->index(row, 0), mappingValue.value("old").toString());
        m_remapModel->setData(m_remapModel->index(row, 1), mappingValue.value("new").toString());
        row++;
    }

    return true;
}

QJsonObject SymbolRemapperForm::parameters()
{
    QJsonObject params;

    // Pull data from the input fields and input them into pluginState
    QJsonArray mappingsArray;
    for (QPair<QString, QString> pair : m_remapModel->getMappings()) {
        QJsonObject pairValue;
        pairValue.insert("old", pair.first);
        pairValue.insert("new", pair.second);
        mappingsArray.append(pairValue);
    }

    params.insert("mappings", mappingsArray);

    return params;
}
