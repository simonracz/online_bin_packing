function XY_exp = expand(XY, number_of_nodes, dimension)
%EXPAND Expands the input with the harmonic mean of each node and job resources.
%   XY_exp = expand(XY, number_of_nodes, dimension) outputs the expanded XY matrix.

XY_exp = zeros(size(XY, 1), size(XY, 2) + number_of_nodes * 2);
HM = zeros(size(XY,1), number_of_nodes * 2);

% mean(XY(:, dimension), "h", 2)

for i = 1:(number_of_nodes * 2)
	HM(:,i) = mean(XY(:, (1 + (i - 1) * dimension):(i * dimension)), 'h', 2);
end

XY_exp = [HM, XY];

end
