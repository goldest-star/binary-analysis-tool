#include "batchrunner.h"
#include "pluginactionmanager.h"

BatchRunner::BatchRunner() :
    m_id(QUuid::createUuid()),
    m_cancelled(false),
    m_running(false)
{

}

QList<QPair<QUuid, int>> BatchRunner::getStepInputs(QSharedPointer<const PluginActionBatch::ActionStep> step) const
{
    if (m_trueStepInputs.contains(step)) {
        return m_trueStepInputs.value(step);
    }
    else {
        return step->inputs;
    }
}

QSharedPointer<BatchRunner> BatchRunner::create(
        QSharedPointer<PluginActionBatch> batch,
        QList<QSharedPointer<BitContainer>> inputContainers)
{
    auto runner = QSharedPointer<BatchRunner>(new BatchRunner());
    runner->m_batch = batch;
    runner->m_inputContainers = inputContainers;


    for (auto step: batch->actionSteps()) {
        QList<QPair<QUuid, int>> trueInputs;
        for (auto input: step->inputs) {
            if (input.first.isNull()) {
                QUuid specialInput = QUuid::createUuid();
                runner->m_stepOutputs.insert(specialInput, {inputContainers.takeFirst()});
                trueInputs.append({specialInput, 0});
            }
            else {
                trueInputs.append(input);
            }
        }
        runner->m_trueStepInputs.insert(step, trueInputs);
    }

    return runner;
}

QUuid BatchRunner::id() const
{
    return m_id;
}

QStringList BatchRunner::errorList() const
{
    return m_errorList;
}

void BatchRunner::run(QSharedPointer<PluginActionManager> actionManager)
{
    m_actionManager = actionManager;
    if (m_actionManager.isNull()) {
        m_errorList.append("Null action manager");
        emit finished(m_id);
        return;
    }
    m_running = true;
    checkForRunnableSteps();
}


void BatchRunner::cancel()
{
    m_cancelled = true;
    checkDone();
}

void BatchRunner::checkDone()
{
    // check for running jobs then finish
    if (m_analyzerRunners.isEmpty() && m_operatorRunners.isEmpty()) {
        m_running = false;
        emit finished(m_id);
    }
}

void BatchRunner::checkFinishedAnalyzer(QUuid id)
{
    // !TODO: There are some issues that could happen if subsequent operator steps use the containers that exist
    // before the analysis steps for a particular container
    auto finished = m_analyzerRunners.value(id);
    if (finished.first.isNull()) {
        m_errorList.append(QString("Unexpected analyzer step ID finished: %1").arg(id.toString()));
    }
    else {
        auto result = finished.second->getWatcher()->result();
        if (!result->errorString().isEmpty()) {
            m_errorList.append("Analyzer step failed: " + result->errorString());
            this->cancel();
            return;
        }

        m_stepOutputs.insert(finished.first, {finished.second->container()});
    }

    checkForRunnableSteps();
}

void BatchRunner::checkFinishedOperator(QUuid id)
{
    auto finished = m_operatorRunners.take(id);
    if (finished.first.isNull()) {
        m_errorList.append(QString("Unexpected operator step ID finished: %1").arg(id.toString()));
    }
    else {
        auto result = finished.second->getWatcher()->result();
        if (!result->errorString().isEmpty()) {
            m_errorList.append("Operator step failed: " + result->errorString());
            this->cancel();
            return;
        }

        m_stepOutputs.insert(finished.first, result->getOutputContainers());
    }

    checkForRunnableSteps();
}

void BatchRunner::checkForRunnableSteps()
{
    // check if the batch runner is cancelled
    if (m_cancelled) {
        checkDone();
        return;
    }

    // check if there are any more steps to run
    if (m_batch->actionSteps().size() == m_ranSteps.size()) {
        checkDone();
        return;
    }

    // Figure out which steps have available inputs
    QList<QSharedPointer<const PluginActionBatch::ActionStep>> runnableSteps;
    for (auto step: m_batch->actionSteps()) {
        if (m_ranSteps.contains(step->stepId)) {
            continue;
        }
        bool hasInputs = true;
        for (auto input: getStepInputs(step)) {
            if (!m_stepOutputs.contains(input.first)) {
                hasInputs = false;
                break;
            }
        }
        if (hasInputs) {
            runnableSteps.append(step);
        }
    }

    // Run each step with available inputs
    bool hasNewImports = false;
    for (auto step: runnableSteps) {
        m_ranSteps.insert(step->stepId);
        QList<QSharedPointer<BitContainer>> stepInputs;
        for (auto input: getStepInputs(step)) {
            auto source = m_stepOutputs.value(input.first);
            if (input.second > source.size()) {
                m_errorList.append(QString("Outputs of %1 did not contain a value for %2 as expected").arg(input.first.toString()).arg(step->stepId.toString()));
                cancel();
                return;
            }
            stepInputs.append(source.at(input.second));
        }
        if (step->action->getPluginType() == PluginAction::NoAction) {
            m_stepOutputs.insert(step->stepId, stepInputs);
            hasNewImports = true;
        }
        else if (step->action->getPluginType() == PluginAction::Importer) {
            auto result = m_actionManager->runImporter(step->action);
            if (!result->errorString().isEmpty()) {
                m_errorList.append(result->errorString());
                cancel();
                return;
            }
            if (result->getContainer().isNull()) {
                m_errorList.append("Importer did not import anything, cancelling batch");
                cancel();
                return;
            }
            m_stepOutputs.insert(step->stepId, {result->getContainer()});
            hasNewImports = true;
        }
        else if (step->action->getPluginType() == PluginAction::Exporter) {
            if (stepInputs.size() == 1) {
                auto result = m_actionManager->runExporter(step->action, stepInputs.at(0));
                if (!result->errorString().isEmpty()) {
                    m_errorList.append("Export step failed: " + result->errorString());
                }
            }
            else {
                m_errorList.append("Export step failed - a single input container is required");
            }
        }
        else if (step->action->getPluginType() == PluginAction::Analyzer) {
            if (stepInputs.size() != 1) {
                m_errorList.append(QString("Invalid input specification for analysis step %1").arg(step->stepId.toString()));
                cancel();
                return;
            }
            auto runner = m_actionManager->runAnalyzer(step->action, stepInputs.at(0));
            m_analyzerRunners.insert(runner->id(), {step->stepId, runner});
            connect(runner.data(), &AnalyzerRunner::finished, this, &BatchRunner::checkFinishedAnalyzer);
        }
        else if (step->action->getPluginType() == PluginAction::Operator) {
            auto runner = m_actionManager->runOperator(step->action, stepInputs);
            m_operatorRunners.insert(runner->id(), {step->stepId, runner});
            connect(runner.data(), &OperatorRunner::finished, this, &BatchRunner::checkFinishedOperator);
        }

    }

    // check if the batch runner is un-runnable due to missing inputs or expected step outputs
    if (runnableSteps.isEmpty()
            && m_batch->actionSteps().size() > m_ranSteps.size()
            && m_analyzerRunners.isEmpty()
            && m_operatorRunners.isEmpty()) {
        m_errorList.append("There are steps that cannot run due to missing inputs or missing expected output from other steps");
        cancel();
    }

    // if a new container was synchronously imported, immediately check again if other steps can be taken with it
    if (hasNewImports) {
        checkForRunnableSteps();
    }
}
