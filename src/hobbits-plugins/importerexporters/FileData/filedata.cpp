#include "filedata.h"
#include "settingsmanager.h"
#include "ui_filedata.h"
#include <QFileDialog>
#include <QMessageBox>

FileData::FileData() :
    ui(new Ui::FileData())
{

}

FileData::~FileData()
{
    delete ui;
}

ImportExportInterface* FileData::createDefaultImporterExporter()
{
    return new FileData();
}

QString FileData::getName()
{
    return "File Data";
}

bool FileData::canExport()
{
    return true;
}

bool FileData::canImport()
{
    return true;
}

QString FileData::getImportLabelForState(QJsonObject pluginState)
{
    if (pluginState.contains("filename")) {
        QString fileName = pluginState.value("filename").toString();
        return QString("Import file %1").arg(fileName);
    }
    return "";
}

QString FileData::getExportLabelForState(QJsonObject pluginState)
{
    Q_UNUSED(pluginState)
    return "";
}

QSharedPointer<ImportResult> FileData::importBits(QJsonObject pluginState)
{
    QString fileName;

    if (pluginState.contains("filename")) {
        fileName = pluginState.value("filename").toString();
    }
    else {
        fileName = QFileDialog::getOpenFileName(
                nullptr,
                tr("Import Bits"),
                SettingsManager::getInstance().getPrivateSetting(SettingsData::LAST_IMPORT_EXPORT_PATH_KEY).toString(),
                tr("All Files (*)"));
    }

    if (fileName.isEmpty()) {
        return ImportResult::error("No file selected for import");
    }

    pluginState.remove("filename");
    pluginState.insert("filename", fileName);

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox msg;
        msg.setWindowTitle("Import Bits Error");
        msg.setText(QString("Failed to import bit file: '%1'").arg(fileName));
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return ImportResult::error(QString("Failed to import bit file: '%1'").arg(fileName));
    }
    SettingsManager::getInstance().setPrivateSetting(
            SettingsData::LAST_IMPORT_EXPORT_PATH_KEY,
            QFileInfo(file).dir().path());

    QSharedPointer<BitContainer> container = QSharedPointer<BitContainer>(new BitContainer);
    container->setBits(&file);
    container->setName(QFileInfo(file).fileName());

    return ImportResult::result(container, pluginState);
}

QSharedPointer<ExportResult> FileData::exportBits(QSharedPointer<const BitContainer> container, QJsonObject pluginState)
{
    QString fileName;

    if (pluginState.contains("filename")) {
        fileName = pluginState.value("filename").toString();
    }
    else {
        fileName = QFileDialog::getSaveFileName(
                nullptr,
                tr("Export Bits"),
                SettingsManager::getInstance().getPrivateSetting(SettingsData::LAST_IMPORT_EXPORT_PATH_KEY).toString(),
                tr("All Files (*)"));
    }
    pluginState.remove("filename");
    pluginState.insert("filename", fileName);

    QFile file(fileName);
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        return ExportResult::error(QString("Failed to open export bit file: '%1'").arg(fileName));
    }
    SettingsManager::getInstance().setPrivateSetting(
            SettingsData::LAST_IMPORT_EXPORT_PATH_KEY,
            QFileInfo(file).dir().path());

    container->bits()->writeTo(&file);
    file.close();

    return ExportResult::result(pluginState);
}
