import networkx as nx
import matplotlib.pyplot as plt

G = nx.Graph()

with open('adj.txt', 'r') as f:
    for node, line in enumerate(f): 
        neighbors = list(map(int, line.strip().split()))
        for neighbor in neighbors:
            if neighbor != node: 
                G.add_edge(node, neighbor)

pos = nx.spring_layout(G, seed=42)
nx.draw(G, pos, with_labels=True, node_color='lightblue', node_size=1000, font_size=12)
plt.title("Undirected Graph from Adjacency List (Line Number = Node ID)")
plt.show()
