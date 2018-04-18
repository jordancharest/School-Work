%% Post-processing the final world state of the gam eof life simuation into a heatmap
file = fopen('final_world_state.txt');
world_size = 1024;
rf = 16;

data = fread(file, [world_size world_size]);
heatmap_data = zeros(world_size/rf, world_size/rf);

pooled_sum = 0;


% pool the data together into a smaller format
for i = 1:world_size/rf
    for j = 1:world_size/rf
        pooled_sum = 0; 
        
        for m = (i-1)*rf+1:(i-1)*rf+rf
            for n = (j-1)*rf+1:(j-1)*rf+rf
                pooled_sum = pooled_sum + data(m,n);
            end
        end
       heatmap_data(i,j) = pooled_sum;
    end
end

h = imagesc(heatmap_data);
