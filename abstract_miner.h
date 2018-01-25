#include "stratum.h"

class AbstractMiner {
  public:
    AbstractMiner(int thread_num);
    virtual bool started() = 0;
    virtual void mine(JobPtr job) = 0;
    void sendResult(unsigned char *extranonce2, unsigned char *ntime, unsigned char *nonce);
    virtual void make_workdata(int thr_id, work *wrk) = 0;
    virtual bool do_work(int thr_id, work *wrk) = 0;
    JobPtr job;
};

class ThreadArgs {
    int thr_id;
    AbstractMiner *miner;
};

