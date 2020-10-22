#include "pythonimporter.h"
#include <QTemporaryDir>
#include "hobbitspython.h"

PythonImporter::PythonImporter(QSharedPointer<PythonPluginConfig> config) :
    m_config(config)
{

}

ImporterExporterInterface *PythonImporter::createDefaultImporterExporter()
{
    return new PythonImporter(m_config);
}

QString PythonImporter::name()
{
    return m_config->name();
}

QString PythonImporter::description()
{
    return m_config->description();
}

QStringList PythonImporter::tags()
{
    return m_config->tags();
}

bool PythonImporter::canExport()
{
    return false;
}

bool PythonImporter::canImport()
{
    return true;
}

QSharedPointer<ParameterDelegate> PythonImporter::importParameterDelegate()
{
    return m_config->delegate();
}

QSharedPointer<ParameterDelegate> PythonImporter::exportParameterDelegate()
{
    return nullptr;
}

QSharedPointer<ImportResult> PythonImporter::importBits(QJsonObject parameters, QSharedPointer<PluginActionProgress> progress)
{
    if (!m_config->delegate()->validate(parameters)) {
        return ImportResult::error("Invalid plugin parameters");
    }

    QTemporaryDir dir;
    if (!dir.isValid()) {
        return ImportResult::error("Could not create temporary directory");
    }

    QFile userScriptFile(dir.filePath("user_script.py"));
    if (!userScriptFile.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        return ImportResult::error("Could not write script to temporary directory");
    }
    userScriptFile.write(m_config->script().toLatin1());
    userScriptFile.close();

    auto outputBits = QSharedPointer<BitArray>(new BitArray());
    auto outputInfo = BitInfo::create();
    auto pyRequest = PythonRequest::create(userScriptFile.fileName())->setFunctionName("import_bits");
    for (auto extraPath: m_config->extraPaths()) {
        pyRequest->addPathExtension(extraPath);
    }
    pyRequest->addArg(PythonArg::bitArray(outputBits));
    pyRequest->addArg(PythonArg::bitInfo(outputInfo));
    for (auto param : m_config->parameterInfos()) {
        if (param.type == QJsonValue::String) {
            pyRequest->addArg(PythonArg::qString(parameters.value(param.name).toString()));
        }
        else if (param.type == QJsonValue::Double) {
            if (m_config->parameterNumberType(param.name) == PythonPluginConfig::Integer) {
                pyRequest->addArg(PythonArg::integer(parameters.value(param.name).toInt()));
            }
            else {
                pyRequest->addArg(PythonArg::number(parameters.value(param.name).toDouble()));
            }
        }
        else if (param.type == QJsonValue::Bool) {
            pyRequest->addArg(PythonArg::boolean(parameters.value(param.name).toBool()));
        }
    }

    auto watcher = HobbitsPython::getInstance().runProcessScript(pyRequest, progress);
    watcher->watcher()->future().waitForFinished();
    auto result = watcher->result();

    QString output = "";
    bool error = false;
    if (!result->getStdOut().isEmpty()) {
        output += "Python stdout:\n" + result->getStdOut() + "\n\n";
    }
    if (!result->getStdErr().isEmpty()) {
        error = true;
        output += "Python stderr:\n" + result->getStdErr() + "\n\n";
    }
    if (!result->errors().isEmpty()) {
        error = true;
        output += "Other errors:\n" + result->errors().join("\n") + "\n\n";
    }
    if (error) {
        return ImportResult::error(output);
    }

    outputInfo->setMetadata(name() + " Run Output", output);

    QSharedPointer<BitContainer> importerContainer = BitContainer::create(outputBits);
    importerContainer->setInfo(outputInfo);
    importerContainer->setName(QString("%1 Import").arg(name()));

    return ImportResult::result(importerContainer, parameters);
}

QSharedPointer<ExportResult> PythonImporter::exportBits(QSharedPointer<const BitContainer> container, QJsonObject parameters, QSharedPointer<PluginActionProgress> progress)
{
    Q_UNUSED(container)
    Q_UNUSED(parameters)
    Q_UNUSED(progress)
    return ExportResult::error("Export not implemented");
}
