%% High Performance Computing
% Performance Analysis for Homework 4/5
% Jordan Charest

c = categorical({'5 - Ranks: 256, Threads: 0', '4 - Ranks: 64, Threads: 4', '3 - Ranks: 16, Threads: 16', '2 - Ranks: 8, Threads: 64', '1 - Ranks: 4, Threads: 64'});
nodes4 = [95.270125, 95.537918, 112.684935, 118.168217, 129.901324];
nodes16 = [53.983557, 54.540438, 56.819940, 73.131786, 83.427428];
nodes64 = [53.945223, 54.499864, 56.818584, 72.919329, 82.972840];
nodes128 = [53.938134, 54.497061, 56.818459, 72.132025, 80.718423];
nodes128_IO = [2.262167, 3.070058, 3.277328, 3.760902, 4.297163];

hold on
 plot(c, nodes4, '-o', 'linewidth', 1.5)
 plot(c, nodes16, '-*', 'linewidth', 1.5)
 plot(c, nodes64, '-^', 'linewidth', 1.5);
 title('Compute Time Performance Analysis');
 ylabel('Execution time - s');
 xlabel('Run parameters');