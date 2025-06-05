#include<bits/stdc++.h>
using namespace std;
#define printdp(v) for(int i=0;i<v.size();i++){for(int j=0;j<v[i].size();j++){cout<<v[i][j]<<" ";}cout<<endl;}

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


int main(){
    int a = 50;
    int b = 100;
    srand(time(0));
    int n = a + rand()%(b-a+1); 

    Network p2p(n);
    vector<vector<int>> adj = p2p.generategraph();

    printdp(adj);

}