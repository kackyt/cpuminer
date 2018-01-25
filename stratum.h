/**
 * Stratum protocol header for GapMiner
 *
 * Copyright (C)  2014  The Gapcoin developers  <info@gapcoin.org>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  
 * Messages send by clients:
 *
 *   Getwork request:
 *     "{ "id": <request-id>, "method": "blockchain.block.request", "params":
 *        { "user": "<user>", "password": "<password" } }"
 *  
 *   Sendwork request:
 *     "{ "id": <share-id>, "method": "blockchain.block.submit", "params": 
 *        { "user": "<user>", "password": "<password", 
 *          "data": "<block hex data>"} }"
 *  
 *
 * Messages send by server:
 *
 *   Block notification:
 *     "{ "id": null, "method": "blockchain.block.new", "params": 
 *        { "data": <block hex data to solve>, 
 *          "difficulty": <target difficulty> } }"
 *  
 *     This message informs the clients that a new block arrived 
 *     and that they should update their mining work.
 *  
 *   Getwork response:
 *     "{ "id": <id of the request>, "result": { "data": <block data to solve>,    
 *        "difficulty": <target difficulty> }, "error": <null or errors string> }"
 *  
 *     This message send the current block data when the client asks for a
 *     work update.
 *  
 *   Sendwork response:
 *     "{ "id": <id of the share>, "result": <true/false>,
 *        "error": <null or errors string>}"
 *  
 *
 *  Note: all messages have to end with a new line character: \n
 *        When a client connects, or reconnects, it first sends
 *        a getwork request.
 */
#include <iostream>
#include <pthread.h>
#include <map>
#include <vector>
#include <memory>
#include <rapidjson/document.h>

using namespace std;

struct Job {
  public:
    std::string job_id;
    unsigned char prevhash[32];
    size_t coinbase_size;
    std::unique_ptr<unsigned char[]> coinbase;
    unsigned char *xnonce2;
    int merkle_count;
    std::vector<std::unique_ptr<unsigned char[]>>merkles;
    unsigned char version[4];
    unsigned char nbits[4];
    unsigned char ntime[4];
    bool clean;
    double difficulty;
};

using JobPtr = std::shared_ptr<Job>;

class AbstractMiner;

#define TWO_POW48 (((uint64_t) 1) << 48)

class Stratum {
  public:

    /* access or create the only instance of this */
    static Stratum *get_instance(const char *host = NULL, 
                                 const char *port = NULL, 
                                 const char *user = NULL,
                                 const char *password = NULL,
                                 uint16_t shift = 0,
                                 AbstractMiner *miner = NULL);


    /* stop this */
    void stop();

    void send_authorize();
    void send_subscribe();

    /**
     * Thread that listens for new messages from the server.
     * it updates miners, and prints share information
     *
     * Messages that can be received by this:
     *
     * Block notification:
     *   "{ "id": null, "method": "blockchain.block.new", "params": 
     *      { "data": <block hex data to solve>, 
     *        "difficulty": <target difficulty> } }"
     *
     *   This message informs the clients that a new block arrived 
     *   and that they should update their mining work.
     *
     * Getwork response:
     *   "{ "id": <id of the request>, "result": { "data": <block data to solve>,    
     *      "difficulty": <target difficulty> }, "error": <null or errors string> }"
     *
     *   This message send the current block data when the client asks for a
     *   work update.
     *
     * Sendwork response:
     *   "{ "id": <id of the share>, "result": <true/false>,
     *      "error": <null or errors string>}"
     *
     *   This message sends the result for a submitted share, which can be
     *   accepted (true) ore stale (false)
     * 
     * Note: "error" is currently ignored.
     */
    static void *recv_thread(void *arg);


  private:

    /* creates a new Stratum instance */
    Stratum(const char *host = NULL,
            const char *port = NULL,
            const char *user = NULL,
            const char *password = NULL,
            uint16_t shift = 0,
            AbstractMiner *miner = NULL);

    ~Stratum();

    /* class holding info for the recv_thread */
    class ThreadArgs {

      public :
        Stratum *client;
        AbstractMiner *miner;
        map<int, double> *shares;
        bool running;

        ThreadArgs(Stratum *client, AbstractMiner *miner, map<int, double> *shares);
    };

    /* synchronization mutexes */
    static pthread_mutex_t creation_mutex;
    static pthread_mutex_t connect_mutex;
    static pthread_mutex_t send_mutex;
    static pthread_mutex_t shares_mutex;

    /* helper function which processes an response share */
    void process_share(map<int, double> *shares, int id, bool accepted);

    /**
     * (re)start an keep alive tcp connection
     */
    void reinit();

    void parse_notify(const rapidjson::Value &params);
    /**
     * (re)connect to a given addr
     */
    bool connect_to_addr(struct addrinfo *addr);

    /**
     * (re)connect to a given pool
     */
    void reconnect();

    /* the server address */
    string host;

    /* the server port */
    string port;

    /* the user */
    string user;

    /* the users password */
    string password;

    /* the mining shift */
    uint16_t shift;

    /* the only instance of this */
    static Stratum *only_instance;

    /* the thread args of this */
    ThreadArgs *targs;

    /* waiting share vector */
    map<int, double> shares;

    /* message counter */
    int n_msgs;

    /* indicates that this is running */
    bool running;

  public:
    /* the socket of this */
    int tcp_socket;

    /* thread object of this */
    pthread_t thread;

    JobPtr current_job;

    std::unique_ptr<unsigned char[]> xnonce1;
    size_t xnonce1_size;
    size_t xnonce2_size;
    double difficulty;
};
