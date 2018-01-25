#include "abstract_miner.h"

static void *
miner_thread(void *a)
{
    ThreadArgs *args = (ThreadArgs*)a;
    int thr_id = args->thr_id;
    AbstractMiner *miner = args->miner;
    work wrk;
    JobPtr current_job = nullptr;

    for (;;) {
        if (current_job != miner->job) {
            miner->make_workdata(thr_id, &wrk);
            current_job = miner->job;
        }

        if (do_work(thr_id, &wrk)) {
            /* sendResult */
        }
    }

    return nullptr;
}
