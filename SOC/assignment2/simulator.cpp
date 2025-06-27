#include<bits/stdc++.h>
using namespace std;
#define printdp(v) for(int i=0;i<v.size();i++){for(int j=0;j<v[i].size();j++){cout<<v[i][j]<<" ";}cout<<endl;}
typedef long long ll;

const int TRANSACTION_SIZE_BITES = 8*1024;
static ll timestamp = 0;
const int MINE_REWARD = 500000;
const int REWARD_ID = -1;
const string EMPTY_STR = "";
int MAX_BLOCK_SIZE = 100;
int MIN_TXN_AMOUNT = 1;

// TODO: initilize constants

int txn_exp_mean_time = 2000;
int AVG_BLOCK_GEN_TIME = 600000;

class Simulator;

class Transaction{
    private:
        string txn_id;
        int sender_id;
        int reciver_id;
        int txn_amount;
        ll txn_local_timestamp;

        static atomic<long long> nxt_txn_id;

    public:
        Transaction() = default;

        Transaction(int from_id, int to_id, int txn_amount) : sender_id(from_id),
                     reciver_id(to_id), txn_amount(txn_amount), txn_local_timestamp(timestamp) {
            txn_id = "Txn-" + to_string(nxt_txn_id++);
        }

        const string get_txn_id() const{
            return txn_id;
        } 

        const int get_sender_id() const{
            return sender_id;
        }

        const int get_reciver_id() const{
            return reciver_id;
        }

        const int get_txn_amount() const{
            return txn_amount;
        }

        ll get_txn_local_timestamp(){
            return txn_local_timestamp;
        }

        void update_txn_timestamp(int &latency){
            txn_local_timestamp += latency;
        }

};

atomic<long long> Transaction::nxt_txn_id = 0;

class Block{
    private:
        string block_id;
        string prev_block_id;
        vector<Transaction> transactions;
        int miner_id;
        int size_kb;
        ll block_mine_local_timestamp;

        static atomic<long long> nxt_block_id;
        
    public:
        Block(){
            block_id = "Blk-" + to_string(nxt_block_id++);
        };

        Block(string prev_block_id, int miner_id) : prev_block_id(prev_block_id), miner_id(miner_id){
            Transaction mine_reward(REWARD_ID, miner_id, MINE_REWARD);
            transactions.push_back(mine_reward);
            block_id = "Blk-" + to_string(nxt_block_id++);
            size_kb = transactions.size();
            block_mine_local_timestamp = timestamp;
        }

        void add_txn(Transaction &txn){
            transactions.push_back(txn);
        }

        void update_block_timestamp(int &latency){
            block_mine_local_timestamp += latency;
        }

        int get_block_size(){
            return transactions.size();
        }

        vector<Transaction> get_transcations(){
            return transactions;
        }

        string get_block_id(){
            return block_id;
        }
        
        string get_prev_block_id(){
            return prev_block_id;
        }

        ll get_blk_timestamp(){
            return block_mine_local_timestamp;
        }

};

atomic<long long> Block::nxt_block_id = 0;

Block genesis_blk;

struct BlockNode{
    Block block;
    BlockNode* parent;
    vector<BlockNode*> children;
    ll arrival_time;
    int depth;

    BlockNode(Block blk, BlockNode* parent = nullptr, int depth = 0, ll arrival_time = 0) : block(blk), arrival_time(arrival_time), parent(parent), depth(depth){}
    
};


class BlockChainTree{
    private:
        unordered_map<string, BlockNode*> blocks_map;
        string genesis_block_id;
        string longest_chain_tip_id;
        vector<string> longest_chains;

        void print_full_tree_iterative() {
            BlockNode* root = blocks_map[genesis_block_id];
            if (!root) return;

            cout << root->block.get_block_id() << " (Genesis)" << endl;

            std::stack<std::pair<BlockNode*, string>> s;
            for (int i = root->children.size() - 1; i >= 0; --i) {
                s.push({root->children[i], ""});
            }

            while (!s.empty()) {
                auto [node, prefix] = s.top();
                s.pop();

                cout << prefix << (s.empty() || s.top().second != prefix ? "└──" : "├──");
                cout << node->block.get_block_id() << " (Depth: " << node->depth << ")" << endl;

                string child_prefix = prefix + (s.empty() || s.top().second != prefix ? "    " : "│   ");
                for (int i = node->children.size() - 1; i >= 0; --i) {
                    s.push({node->children[i], child_prefix});
                }
            }
        }
    
        bool validate_block(string parent_id, ll block_timestamp){
            
            return true;
            if(!blocks_map.count(parent_id)){
                return true;
            }

            else{
                for(string &pid : longest_chains){
                    if(pid == parent_id){
                        return true;
                    }
                    if(block_timestamp == blocks_map[pid]->arrival_time){
                        return true;
                    }
                }
                
                return false;
            }
        }

        void update_longest_chain_tip(){
            int iter_height = -1;
            
            for(auto it = blocks_map.begin(); it != blocks_map.end(); it++){
                if(it->second->depth > iter_height){
                    longest_chains.clear();
                    longest_chain_tip_id = it->second->block.get_block_id();
                    longest_chains.push_back(longest_chain_tip_id);
                    iter_height = it->second->depth;
                }
                else if(it->second->depth == iter_height){
                    longest_chain_tip_id = EMPTY_STR;
                    longest_chains.push_back(it->second->block.get_block_id());
                }
            }
        }

        void update_mempool(Block &blk, unordered_map<string, Transaction> &mempool, vector<int> &peers_balances){
            vector<Transaction> txns = blk.get_transcations();
            Transaction coinbase = txns[0];
            peers_balances[coinbase.get_reciver_id()] += coinbase.get_txn_amount();

            for(int i=1;i<txns.size();i++){
                auto it = mempool.find(txns[i].get_txn_id());
                if(it != mempool.end()){
                    int txn_amount = it->second.get_txn_amount();
                    peers_balances[it->second.get_sender_id()] -= txn_amount;
                    peers_balances[it->second.get_reciver_id()] += txn_amount;

                    mempool.erase(it);
                }
            }
        }

        void redo_mempool_transactions(string &tip_id, unordered_map<string, Transaction> &mempool, vector<int> &peers_balances){
            vector<Transaction> txns = blocks_map[tip_id]->block.get_transcations();

            Transaction coinbase = txns[0];
            peers_balances[coinbase.get_reciver_id()] -= coinbase.get_txn_amount();

            for(int i=1;i<txns.size();i++){
                auto it = mempool.find(txns[i].get_txn_id());
                if(it == mempool.end()){
                    int txn_amount = txns[i].get_txn_amount();
                    peers_balances[txns[i].get_sender_id()] += txn_amount;
                    peers_balances[txns[i].get_reciver_id()] -= txn_amount;

                    mempool[txns[i].get_txn_id()] = txns[i];
                }
            }
        }


        void update_self_balance(Block blk, int &self_balance, int peer_id){
            vector<Transaction> txns = blk.get_transcations();

            for(Transaction t : txns){
                if(t.get_reciver_id() == peer_id){
                    self_balance += t.get_txn_amount();
                }
            }
        }

        void redo_self_balance_update(string blk_id, int &self_balance, int peer_id){
            vector<Transaction> txns = blocks_map[blk_id]->block.get_transcations();

            for(Transaction t : txns){
                if(t.get_reciver_id() == peer_id){
                    self_balance -= t.get_txn_amount();
                }
            }
        }


    
    public:

        BlockChainTree(){
            genesis_block_id = genesis_blk.get_block_id();
            longest_chain_tip_id = genesis_block_id;
            BlockNode* genesis_node = new BlockNode(genesis_blk);
            blocks_map[genesis_block_id] = genesis_node;
        }

        bool add_block(Block new_block, unordered_map<string, Transaction> &mempool, vector<int> &peers_balances, Simulator* sim, int &self_balance, int peer_id);

        const string get_longest_chain_id() const{
            return longest_chain_tip_id;
        }

        BlockNode* get_blocknode(string block_id){
            auto it = blocks_map.find(block_id);

            if(it != blocks_map.end()){
                return it->second;
            }

            return nullptr;
        }

        string get_genesis_block_id(){
            return genesis_block_id;
        }

        void print_full_tree() {
            // cout << blocks_map[genesis_block_id]->block.get_block_id() << " (Genesis)" << endl;
            print_full_tree_iterative();
        }

};

enum class EventType{

    UPADTE_BLOCKCHAIN,
    BRODCAST_TXN,
    RECIVED_TXN

};

struct Event{

    ll timestamp;
    EventType type;
    int peer_id;
    int size;
    
    shared_ptr<void> data_payload = nullptr;
};

struct EventComprator {

    bool operator()(const Event &a, const Event &b) const{
        if(a.timestamp != b.timestamp){
            return a.timestamp > b.timestamp;
        }

        else{
            return static_cast<int>(a.type) > static_cast<int>(b.type);
        }
    }
};

class Peer{
    private:
        int peer_id;
        vector<int> adj;
        int balance;
        ll nxt_txn_timestamp;
        bool fast_internet;
        bool is_mining;
        double hashing_power;
        ll mining_complete_time;
        int n_peers;
        
        BlockChainTree block_chain;
        unordered_map<string, Transaction> mempool;
        vector<Block> upcoming_blocks;
        Block mined_block;
        vector<int> peers_balances;
        Simulator* sim;

        mt19937 gen;
        
        Transaction generate_transaction(){
            int sender_id = peer_id;
            int reciver_id;

            uniform_int_distribution<> dist(0,n_peers-1);
            reciver_id = dist(gen);

            while(reciver_id == sender_id){
                reciver_id = dist(gen);
            }

            uniform_int_distribution<> real_dist(MIN_TXN_AMOUNT, balance);
            int transfer_amount = real_dist(gen);

            Transaction new_txn(sender_id, reciver_id, transfer_amount);

            return new_txn;
        }

        void update_next_transaction_timestamp();

        int time_to_mine_block(){
            double lambda = hashing_power*1.0/AVG_BLOCK_GEN_TIME;

            exponential_distribution<> exp_dist(lambda);
            int sample = exp_dist(gen);

            return sample;
        }


    public:
        Peer() = default;

        Peer(int id, vector<int> adj, int balance, bool internet, double hash_power, vector<int> peers_balances, Simulator* sim) : peer_id(id), adj(adj),
                 balance(balance), fast_internet(internet), hashing_power(hash_power), peers_balances(peers_balances), sim(sim){
            
            is_mining = false;
            update_next_transaction_timestamp();
            mining_complete_time = -1;
            random_device rd;
            gen = mt19937(rd());
            n_peers = peers_balances.size();
        }

        int get_peer_id(){
            return peer_id;
        }

        int get_balance(){
            return balance;
        }

        bool get_internet_speed(){
            return fast_internet;
        }

        void add_txn_mempool(Transaction new_txn){
            mempool[new_txn.get_txn_id()] = new_txn;
        }
        void start_block_mining();

        bool do_transaction(Event event);

        void update_upcoming_blocks(Block new_block){
            if(upcoming_blocks.empty()){
                upcoming_blocks.push_back(new_block);
            }
            else{
                if(new_block.get_blk_timestamp() == upcoming_blocks[0].get_blk_timestamp()){
                    upcoming_blocks.push_back(new_block);
                }

                else if(new_block.get_blk_timestamp() < upcoming_blocks[0].get_blk_timestamp()){
                    upcoming_blocks.clear();
                    upcoming_blocks.push_back(new_block);
                }
            }
        }

        void update_block_chain(){
            if(upcoming_blocks.empty() && timestamp != mining_complete_time){
                return;
            }

            if(timestamp == mining_complete_time){
                // successful mining

                block_chain.add_block(mined_block, mempool, peers_balances, sim, balance, peer_id);
                if(!upcoming_blocks.empty()){
                    int i = 0;
                    int n = upcoming_blocks.size();

                    while(i<n && upcoming_blocks[i].get_blk_timestamp() == timestamp){
                        block_chain.add_block(upcoming_blocks[i], mempool, peers_balances, sim, balance, peer_id);
                        i++;
                    }

                }

                upcoming_blocks.clear();
                
                is_mining = false;
                mining_complete_time = -1;
                start_block_mining();
            }


            else if(timestamp == upcoming_blocks[0].get_blk_timestamp()){
                // unsuccessful mining

                int i = 0;
                int n = upcoming_blocks.size();
                bool is_added = false;

                while(i<n && upcoming_blocks[i].get_blk_timestamp() == timestamp){
                    is_added = is_added | block_chain.add_block(upcoming_blocks[i], mempool, peers_balances, sim, balance, peer_id);
                    i++;
                }

                if(is_added){
                    is_mining = false;
                    mining_complete_time = -1;
                    start_block_mining();
                }

                upcoming_blocks.clear();
            }

        }

        bool is_block_intree(string block_id){
            BlockNode* find_node = block_chain.get_blocknode(block_id);

            if(find_node != nullptr){
                return true;
            }
            return false;
        }

        Block give_block_intree(string block_id){
            BlockNode* find_node = block_chain.get_blocknode(block_id);
            return find_node->block;
        }

        vector<int> return_peers_balances(){
            return peers_balances;
        }

        void print_blockchain_tree() {
            cout << "\n--- Blockchain Tree for Peer " << this->peer_id << " ---" << endl;
            this->block_chain.print_full_tree();
            cout << "------------------------------------------" << endl;
        }

};

class Network{
    private:
        vector<int> node_order;
        vector<vector<int>> adj;
        int n;

        void generate_node_order(){
            srand(time(0));
    
            for(int i =0;i<n;i++){
                node_order[i] = i;
            }
    
            for(int i =n-1;i>0;i--){
                int j = rand()%(i);
                swap(node_order[i], node_order[j]);        
            }
        }
    
    public: 
        Network() = default;

        Network(int &y) : n(y){
            node_order.resize(n);
            adj.resize(n);
        };


        vector<vector<int>> generategraph(){
            generate_node_order();
            int ptr = 1;
            vector<int> present;
            vector<int> degree(n,0);
            vector<unordered_set<int>>us_adj(n);
            queue<int>q;
            q.push(node_order[0]);
            present.push_back(node_order[0]);

            while(!q.empty()){
                int node = q.front();
                q.pop();
                int k = max(0,3-degree[node] + rand()%(4));
                int m = (present.size()-(degree[node]+1));
                int from_present = min(m, k/2);
                int extras = k - from_present;
                if(extras+ptr > n){
                    from_present += extras - (n-ptr);
                    extras = n-ptr;
                }

                //from present
                int cnt = 0;
                while(cnt != from_present){
                    int j = rand()%present.size();
                    if(!us_adj[node].count(present[j]) && degree[present[j]] < 6){
                        cnt++;
                        adj[node].push_back(present[j]);
                        us_adj[node].insert(present[j]);

                        adj[present[j]].push_back(node);
                        us_adj[present[j]].insert(node);

                        degree[node]++;
                        degree[present[j]]++;
                    } 
                }

                //from extras
                for(int i = ptr; i < extras+ptr; i++){
                    adj[node].push_back(node_order[i]);
                    us_adj[node].insert(node_order[i]);

                    adj[node_order[i]].push_back(node);
                    us_adj[node_order[i]].insert(node);

                    present.push_back(node_order[i]);

                    degree[node]++;
                    degree[node_order[i]]++;

                    q.push(node_order[i]);
                }
                ptr += extras;
            }
            return adj;
        }
};


class Simulator{
    private:
        priority_queue<Event, vector<Event>, EventComprator> event_queue;
        vector<Peer> peers;
        ll sim_timestamp;
        int n_peers;
        vector<unordered_map<int, int>> pij;
        ll ticks;

        vector<vector<int>> adj;

        mt19937 gen;


        void set_internet_fraction(vector<bool> &fast_internet, double & fast_internet_fraction){

            int n = fast_internet.size();
            int num_true = static_cast<int>(fast_internet_fraction * n);

            vector<int> indices(n);
            for (int i = 0; i < n; ++i) indices[i] = i;

            shuffle(indices.begin(), indices.end(), gen);

            for (int i = 0; i < num_true; ++i) {
                fast_internet[indices[i]] = true;
            }
        }

        vector<double> set_hashing_power(int n_peers, double f){
            int num_high = static_cast<int> (n_peers*f);
            int temp = 9*num_high+n_peers;
            double x = 1.0/(9*num_high+n_peers);

            vector<double> hashing_power(n_peers, x);
            vector<int> indices(n_peers);
            for(int i=0;i<n_peers;i++) indices[i] = i;

            shuffle(indices.begin(), indices.end(), gen);

            for(int i=0;i<num_high;i++){
                hashing_power[indices[i]] = 10*x;
            }
            return hashing_power;
        }

        void generate_pijs(){
            uniform_int_distribution<> dist(10, 500);
            
            for(int i=0; i<n_peers; i++){
                for(int & adjnode : adj[i]){
                    int random_no = dist(gen);
                    pij[i][adjnode] = random_no;
                }
            }
        }

        double generate_dij(double link_speed){
            link_speed /= 1e3;
            double lambda = link_speed*1.0/96;
            exponential_distribution<> exp_dist(lambda);
            
            double exp_delay = exp_dist(gen);
            return exp_delay;
        }

        int get_delay(int a, int b, int msg_size){
            int light_delay = pij[a][b];
            bool is_fast = peers[a].get_internet_speed() & peers[b].get_internet_speed();
            double link_speed;
            if(is_fast) link_speed = 100 * 1e6;
            else link_speed = 5 * 1e6;

            double queuing_delay = generate_dij(link_speed);
            double msg_delay = msg_size*1024*8.0/link_speed;

            int delay =  static_cast<int>(1000*(queuing_delay+msg_delay)+light_delay);

            return delay;
        }


    public:
        Simulator() = default;

        Simulator(int n_peers, double fast_internet_fraction, double high_cpu_fraction, int initial_balance, ll ticks) : n_peers(n_peers), sim_timestamp(0), ticks(ticks){

            random_device rd;
            gen = mt19937(rd());

            Network network(n_peers);
            adj = network.generategraph();
            vector<bool> fast_internet(n_peers, false);
            set_internet_fraction(fast_internet, fast_internet_fraction);
            
            vector<double> hashing_power = set_hashing_power(n_peers, high_cpu_fraction);
            vector<int> initial_peers_balances(n_peers, initial_balance);

            peers.reserve(n_peers);
            for(int i=0; i<n_peers; i++){
                peers.emplace_back(i, adj[i], initial_balance, fast_internet[i], hashing_power[i], initial_peers_balances, this);
            }

            pij.resize(n_peers);
            generate_pijs();
            
        }

        void set_event(Event event){
            event_queue.push(event);
        }

        Block find_missing_parent(string parent_id){
            for(int i=0;i<n_peers;i++){
                if(peers[i].is_block_intree(parent_id)){
                    return peers[i].give_block_intree(parent_id);
                }
            }
            Block temp;
            return temp;
        }

        void run(){
            while(!event_queue.empty() && sim_timestamp < ticks){
                Event next_event = event_queue.top();
                event_queue.pop();

                sim_timestamp = next_event.timestamp;
                timestamp = sim_timestamp;

                switch(next_event.type){
                    case EventType::BRODCAST_TXN: {
                        int sender_id = next_event.peer_id;
                        peers[sender_id].do_transaction(next_event);
                        break;
                    }

                    case EventType::RECIVED_TXN: {
                        int reciver_id = next_event.peer_id;
                        peers[reciver_id].start_block_mining();
                        break;
                    }

                    case EventType::UPADTE_BLOCKCHAIN: {
                        int action_id = next_event.peer_id;
                        peers[action_id].update_block_chain();
                        break;
                    }
                }

            }
        }
    
        void brodcast_event(Event event){
            int sender_id = event.peer_id;
            int size_in_kb = event.size;

            vector<int> delay(n_peers, 1e9);
            priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
            delay[sender_id] = 0;
            pq.push({0, sender_id});

            while (!pq.empty()) {
                int d = pq.top().first;
                int node = pq.top().second;
                pq.pop();

                if(d > delay[node]) continue; 

                for(int a_node : adj[node]){

                    int new_delay = delay[node] + get_delay(node, a_node, size_in_kb);
                    if(new_delay < delay[a_node]){ 
                        delay[a_node] = new_delay;
                        pq.push({new_delay, a_node}); 
                    }

                }
            }

            switch(event.type){
                case EventType::BRODCAST_TXN:{
                    auto base_txn_ptr = static_pointer_cast<Transaction>(event.data_payload);
                    Transaction base_txn_copy = *base_txn_ptr;

                    for(int i=0;i<n_peers;i++){
                        if(i != sender_id){
                            Transaction temp = base_txn_copy;
                            temp.update_txn_timestamp(delay[i]);
                            peers[i].add_txn_mempool(temp);
                            Event new_recive_event;
                            new_recive_event.peer_id = i;
                            new_recive_event.size = size_in_kb;
                            new_recive_event.timestamp = temp.get_txn_local_timestamp();
                            new_recive_event.type = EventType::RECIVED_TXN;

                            set_event(new_recive_event);
                        }
                    }
                    break;
                }

                case EventType::UPADTE_BLOCKCHAIN: {
                    auto block_base_ptr = static_pointer_cast<Block>(event.data_payload);
                    Block base_block_copy = *block_base_ptr;

                    for(int i=0;i<n_peers;i++){
                        Block temp = base_block_copy;
                        temp.update_block_timestamp(delay[i]);

                        if(i != sender_id){
                            peers[i].update_upcoming_blocks(temp);
                        }

                        Event new_block_event;
                        new_block_event.peer_id = i;
                        new_block_event.size = temp.get_block_size();
                        new_block_event.timestamp = temp.get_blk_timestamp();
                        new_block_event.type = EventType::UPADTE_BLOCKCHAIN;

                        set_event(new_block_event);
                    }
                    break;
                }
            }

        }

        void print_balances(){
            for(int i=0;i<10;i++){
                vector<int> local_balances = peers[i].return_peers_balances();
                for(int x : local_balances){
                    cout<<x*1.0/10000<<" $BTC | ";
                }
                cout<<"\n";
            }
        }

        void print_peer_blockchaintree(int peer_id){
            peers[peer_id].print_blockchain_tree();
        }
};

bool BlockChainTree::add_block(Block new_block, unordered_map<string, Transaction> &mempool, vector<int> &peers_balances, Simulator* sim, int &self_balance, int peer_id){
    bool is_valid = validate_block(new_block.get_prev_block_id(), new_block.get_blk_timestamp());

    if(is_valid){
        string prev_block_id = new_block.get_prev_block_id();

        
        if(!blocks_map.count(prev_block_id)){
            // ask other peers for this block
            for(string ll_tip : longest_chains){
                redo_self_balance_update(ll_tip, self_balance, peer_id);
            }

            Block new_parent_block = sim->find_missing_parent(prev_block_id); 
            
            string grand_parent_id = new_parent_block.get_prev_block_id();
            BlockNode* grand_parent_node = blocks_map[grand_parent_id];
            int h_gp = grand_parent_node->depth;
            ll parent_block_arrival_time = new_parent_block.get_blk_timestamp();

            BlockNode* new_parent_blocknode = new BlockNode(new_parent_block, grand_parent_node,h_gp+1, parent_block_arrival_time);
            grand_parent_node->children.push_back(new_parent_blocknode);
            blocks_map[new_parent_block.get_block_id()] = new_parent_blocknode;

            update_mempool(new_parent_block, mempool, peers_balances);

            update_longest_chain_tip();

        }

        if(longest_chain_tip_id == EMPTY_STR){
            for(string tip_id : longest_chains){
                redo_mempool_transactions(tip_id, mempool, peers_balances);
            }

            update_mempool(blocks_map[prev_block_id]->block, mempool, peers_balances);
            update_self_balance(blocks_map[prev_block_id]->block, self_balance, peer_id);

        }
        update_mempool(new_block, mempool, peers_balances);

        string block_id = new_block.get_block_id();


        BlockNode* parent_node = blocks_map[prev_block_id];
        int h = parent_node->depth;
        ll blk_arrival_time = new_block.get_blk_timestamp();   

        BlockNode* new_blocknode = new BlockNode(new_block, parent_node, h+1, blk_arrival_time);
        parent_node->children.push_back(new_blocknode);
        blocks_map[block_id] = new_blocknode;

        update_longest_chain_tip();
        if(longest_chain_tip_id != EMPTY_STR){
            update_self_balance(new_block, self_balance, peer_id);
        }
        else{
            if(longest_chains.size() == 2){
                if(longest_chains[0] != new_block.get_block_id()) redo_self_balance_update(longest_chains[0], self_balance, peer_id);
                else redo_self_balance_update(longest_chains[1], self_balance, peer_id);
            }
        }

    }
    

    return is_valid;
}

void Peer::update_next_transaction_timestamp(){
    // record event

    double lambda = 1.0/txn_exp_mean_time;

    exponential_distribution<> exp_dist(lambda);
    int sample = exp_dist(gen);

    nxt_txn_timestamp = timestamp + sample;

    Event new_txn_event;
    new_txn_event.timestamp = nxt_txn_timestamp;
    new_txn_event.peer_id = this->peer_id;
    new_txn_event.type = EventType::BRODCAST_TXN;
    new_txn_event.data_payload = nullptr;
    new_txn_event.size = 1;

    sim->set_event(new_txn_event);

}

void Peer::start_block_mining(){
    if(is_mining) return;

    string longest_chain_id = block_chain.get_longest_chain_id();
    if(longest_chain_id == EMPTY_STR){
        is_mining = false;
        mining_complete_time = -1;
        return;
    }

    Block new_mine_block(longest_chain_id, peer_id);
    
    auto it = mempool.begin();
    while(it != mempool.end() && new_mine_block.get_block_size() < MAX_BLOCK_SIZE){
        if(it->second.get_txn_local_timestamp() <= timestamp && peers_balances[it->second.get_sender_id()] >= it->second.get_txn_amount()){
            new_mine_block.add_txn(it->second);
        }
        it++;
    }

    if(new_mine_block.get_block_size() == 1){
        return;
    }

    int mining_time = time_to_mine_block();
    mined_block = new_mine_block;
    is_mining = true;
    mining_complete_time = timestamp + mining_time;
    mined_block.update_block_timestamp(mining_time);

    auto mined_block_payload_ptr = make_shared<Block>(mined_block);

    Event mining_update;
    mining_update.peer_id = peer_id;
    mining_update.timestamp = mining_complete_time;
    mining_update.size = mined_block.get_block_size();
    mining_update.type = EventType::UPADTE_BLOCKCHAIN;

    mining_update.data_payload = mined_block_payload_ptr;

    sim->brodcast_event(mining_update);
}


bool Peer::do_transaction(Event event){
    if(timestamp == nxt_txn_timestamp){
        if(balance <= MIN_TXN_AMOUNT) return false;
        auto new_random_txn_ptr = make_shared<Transaction>(generate_transaction());

        if(abs(new_random_txn_ptr->get_txn_amount()) <= MIN_TXN_AMOUNT){
            return false;
        }
        add_txn_mempool(*new_random_txn_ptr);
        balance -= new_random_txn_ptr->get_reciver_id();

        event.data_payload = new_random_txn_ptr;

        start_block_mining();
        sim->brodcast_event(event);
        update_next_transaction_timestamp();
        return true;

    }
    return false;
}


int main(){
    int n_peers = 100;
    double fast_internet_fraction = 0.3;
    double high_cpu_fraction = 0.2;
    int initial_balance = 100000;
    ll ticks = 10000000;
    Simulator blockchain_sim(n_peers, fast_internet_fraction, high_cpu_fraction, initial_balance, ticks);
    blockchain_sim.run();
    // blockchain_sim.print_balances();
    blockchain_sim.print_peer_blockchaintree(0);
    blockchain_sim.print_peer_blockchaintree(1);
    blockchain_sim.print_peer_blockchaintree(2);
    blockchain_sim.print_peer_blockchaintree(3);
    blockchain_sim.print_peer_blockchaintree(4);
    blockchain_sim.print_peer_blockchaintree(5);
    blockchain_sim.print_peer_blockchaintree(6);
    blockchain_sim.print_peer_blockchaintree(7);
    blockchain_sim.print_peer_blockchaintree(8);
    blockchain_sim.print_peer_blockchaintree(9);
    
}