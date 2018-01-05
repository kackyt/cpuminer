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
#include <jansson.h>
#include <map>
#include "Miner.h"
#include "BlockHeader.h"

using namespace std;

class Stratum {

  public:

    /* access or create the only instance of this */
    static Stratum *get_instance(string *host = NULL, 
                                 string *port = NULL, 
                                 string *user = NULL,
                                 string *password = NULL,
                                 uint16_t shift = 0,
                                 Miner *miner = NULL);


    /* stop this */
    static void stop();
 
     /**
      * sends a given BlockHeader to the server 
      * with a stratum request, the response should
      * tell if the share was accepted or not.
      *
      * The format should be:
      *   "{ "id": <id of the share>, "result": <true/false>,
      *      "error": <null or errors string> }"
      */
    bool sendwork(BlockHeader *header);
 
    /**
     * request new work, return is always NULL, 
     * because it handles response internally .
     *
     * The stratum response for this request should be: 
     *   "{ "id": <id of the request>, "result": 
     *      { "data": <block hex data to solve>,    
     *        "difficulty": <target difficulty> }, 
     *        "error": <null or errors string> }"
     */
    BlockHeader *getwork();
 
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
    Stratum(Miner *miner);

    ~Stratum();

    /* class holding info for the recv_thread */
    class ThreadArgs {
      
      public :
        Miner *miner;
        map<int, double> *shares;
        bool running;

        ThreadArgs(Miner *miner, map<int, double> *shares);
    };

    /* synchronization mutexes */
    static pthread_mutex_t creation_mutex;
    static pthread_mutex_t connect_mutex;
    static pthread_mutex_t send_mutex;
    static pthread_mutex_t shares_mutex;

    /* helper function which processes an response share */
    static void process_share(map<int, double> *shares, int id, bool accepted);

    /**
     * helper function to parse a json block work in the form of:
     * "{ "data": <block data to solve>, "difficulty": <target difficulty> }"
     */
    static void parse_block_work(Miner *miner, json_t *result);

    /**
     * (re)start an keep alive tcp connection
     */
    static void reinit();

    /**
     * (re)connect to a given addr
     */
    static bool connect_to_addr(struct addrinfo *addr);

    /**
     * (re)connect to a given pool
     */
    static void reconnect();

    /* the socket of this */
    static int tcp_socket;

    /* the server address */
    static string *host;

    /* the server port */
    static string *port;

    /* the user */
    static string *user;

    /* the users password */
    static string *password;

    /* the mining shift */
    static uint16_t shift;

    /* the only instance of this */
    static Stratum *only_instance;

    /* the thread args of this */
    ThreadArgs *targs;

    /* waiting share vector */
    map<int, double> shares;

    /* thread object of this */
    pthread_t thread;

    /* message counter */
    int n_msgs;

    /* indicates that this is running */
    static bool running;
};
