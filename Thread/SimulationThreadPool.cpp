
#include "SimulationThreadPool.h"
#include "SimulationRunner.h"
#include "SimSettings.h"
#include "Random.h"
#include <climits>
#include <QDebug>
#include <QThread>

SimulationThreadPool::SimulationThreadPool(EquipmentDb* equipment_db, SimSettings* sim_settings, NumberCruncher* scaler, QObject* parent):
    QObject(parent),
    equipment_db(equipment_db),
    random(new Random(0, std::numeric_limits<unsigned>::max())),
    sim_settings(sim_settings),
    scaler(scaler),
    running_threads(0)
{
    for (int i = 0; i < sim_settings->get_num_threads_current(); ++i) {
        setup_thread(random->get_roll());
    }
}

SimulationThreadPool::~SimulationThreadPool() {
    delete random;
    for (auto & thread_entry : thread_pool)
        delete thread_entry.second;
}

void SimulationThreadPool::run_sim(const QString &setup_string, bool full_sim, int iterations) {
    assert(running_threads == 0);
    assert(thread_results.empty());

    int iterations_per_thread = static_cast<int>(static_cast<double>(iterations) / active_thread_ids.size());

    for (auto & thread : thread_pool) {
        if (!active_thread_ids.contains(thread.first))
            continue;

        thread_results.append(QPair<unsigned, double>(thread.first, 0.0));
        emit start_simulation(thread.first, setup_string, full_sim, iterations_per_thread);
        ++running_threads;
    }

    assert(running_threads > 0);
    assert(!thread_results.empty());
}

bool SimulationThreadPool::sim_running() const {
    return running_threads > 0;
}

void SimulationThreadPool::scale_number_of_threads() {
    if (sim_running())
        return;

    int thread_diff = sim_settings->get_num_threads_current() - active_thread_ids.size();

    if (thread_diff == 0)
        return;

    if (thread_diff < 0)
        remove_threads(-thread_diff);
    else
        add_threads(thread_diff);

    assert(active_thread_ids.size() == sim_settings->get_num_threads_current());
}

void SimulationThreadPool::add_threads(const int num_to_add) {
    for (int i = 0; i < num_to_add; ++i) {
        if (inactive_thread_ids.empty())
            setup_thread(random->get_roll());
        else {
            active_thread_ids.append(inactive_thread_ids.takeLast());
        }
    }
}

void SimulationThreadPool::remove_threads(const int num_to_remove) {
    assert(num_to_remove < active_thread_ids.size());

    for (int i = 0; i < num_to_remove; ++i) {
        inactive_thread_ids.append(active_thread_ids.takeLast());
    }

    assert(!active_thread_ids.empty());
}

void SimulationThreadPool::setup_thread(const unsigned thread_id) {
    SimulationRunner* runner = new SimulationRunner(thread_id, equipment_db, sim_settings, scaler);
    auto* thread = new QThread(runner);

    connect(this, SIGNAL (start_simulation(unsigned, QString, bool, int)), runner, SLOT (run_sim(unsigned, QString, bool, int)));
    connect(runner, SIGNAL (error(QString, QString)), this, SLOT (error_string(QString, QString)));
    connect(runner, SIGNAL (result()), this, SLOT (thread_finished()));
    connect(thread, SIGNAL (finished()), thread, SLOT (deleteLater()));

    thread_pool.append(QPair<unsigned, QThread*>(thread_id, thread));
    active_thread_ids.append(thread_id);

    runner->moveToThread(thread);
    thread->start();
}

void SimulationThreadPool::error_string(const QString& seed, const QString& error) {
    qDebug() << "thread error" << seed << ": " << error;
    --running_threads;

    if (running_threads == 0) {
        thread_results.clear();
    }
}

void SimulationThreadPool::thread_finished() {
    --running_threads;

    if (running_threads > 0)
        return;

    threads_finished();
}
