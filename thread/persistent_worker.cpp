#include "persistent_worker.hpp"

PersistentWorker::PersistentWorker() {}
PersistentWorker::~PersistentWorker() {}

void PersistentWorker::addTask(const std::string &name,
                               const std::function<void()> &jobToExecute) {}
void PersistentWorker::removeTask(const std::string &name) {}