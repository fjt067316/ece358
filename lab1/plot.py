import pandas as pd
import matplotlib.pyplot as plt

# Read the CSV data into a DataFrame
data = pd.read_csv('output.csv', header=None, names=['Type', 'ro', 'avg', 'idle', 'ploss', 'K'])

# Separate data into infinite and finite data
infinite_data = data[data['Type'] == 'infinite']
finite_data = data[data['Type'] == 'finite']

# Plot avg vs ro and idle vs ro for infinite data
plt.figure(figsize=(12, 6))
plt.subplot(1, 2, 1)
plt.plot(infinite_data['ro'], infinite_data['avg'], marker='o', label='Average') # Add average packet count to plot
plt.xlabel('Ro (Traffic Load)')
plt.ylabel('Average # Packets in Queue')
plt.title('Average # Packets in Queue vs Ro for Infinite Buffer')
plt.grid(True)
plt.legend()

plt.subplot(1, 2, 2)
plt.plot(infinite_data['ro'], infinite_data['idle'], marker='o', label='Idle') # Add idle time to plot
plt.xlabel('Ro (Traffic Load)')
plt.ylabel('Portion Spent Idle')
plt.title('Idle vs Ro for Infinite Buffer')
plt.grid(True)
plt.legend()

plt.tight_layout()
plt.savefig('infinite_data_avg.png')  # Save the plot
plt.show()



# Create a table for infinite data
infinite_table = infinite_data[['Type', 'ro', 'avg', 'idle', 'K']].copy()
# infinite_table.to_csv('infinite_table.csv', index=False)

# Plot avg vs ro, idle vs ro, and ploss vs ro for finite data for each K
K_values = finite_data['K'].unique()

plt.figure(figsize=(12, 6))
for K in K_values:
    subset_data = finite_data[finite_data['K'] == K]
    plt.plot(subset_data['ro'], subset_data['avg'], marker='o', label=f'K={K} Average')
    # plt.plot(subset_data['ro'], subset_data['ploss'], marker='o', label=f'K={K} Packet Loss')

plt.xlabel('Ro (Traffic Load)')
plt.ylabel('Average # Packets in Queue')
plt.title('Average # Packets in Queue vs Ro for Finite Buffer')
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig('finite_data_avg.png')  # Save the plot
plt.show()

plt.figure(figsize=(12, 6))
for K in K_values:
    subset_data = finite_data[finite_data['K'] == K]
    # plt.plot(subset_data['ro'], subset_data['avg'], marker='o', label=f'K={K} Average')
    plt.plot(subset_data['ro'], subset_data['ploss'], marker='o', label=f'K={K} Packet Loss')

plt.xlabel('Ro (Traffic Load)')
plt.ylabel('% Packets Lost')
plt.title('% Packet Loss vs Ro for Finite Buffer')
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig('finite_data_loss.png')  # Save the plot
plt.show()

# for K in K_values:
#     subset_data = finite_data[finite_data['K'] == K]
    
#     plt.figure(figsize=(12, 6))
#     plt.subplot(1, 2, 1)
#     plt.plot(subset_data['ro'], subset_data['avg'], marker='o', label='Average')
#     plt.xlabel('Ro (Traffic Load)')
#     plt.ylabel('Average # Packets in Queue')
#     plt.title(f'Average vs Ro for Finite Data (K={K})')
#     plt.grid(True)
#     plt.legend()

#     plt.subplot(1, 2, 2)
#     plt.plot(subset_data['ro'], subset_data['ploss'], marker='o', label='Lost')
#     plt.xlabel('Ro (Traffic Load)')
#     plt.ylabel('Packets Lost')
#     plt.title(f'Packet Loss vs Ro for Finite Data (K={K})')
#     plt.grid(True)
#     plt.legend()

#     plt.tight_layout()
#     plt.show()

    # Create a table for finite data
    # finite_table = subset_data[['Type', 'ro', 'avg', 'idle', 'ploss']].copy()
    # finite_table.to_csv(f'finite_table_K_{K}.csv', index=False)
