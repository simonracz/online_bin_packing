function p = predict(Theta1, Theta2, number_of_nodes, X)
%PREDICT Predict the label of an input given a trained neural network
%   p = PREDICT(Theta1, Theta2, X) outputs the predicted label of X given the
%   trained weights of a neural network (Theta1, Theta2)

m = size(X, 1);
num_labels = size(Theta2, 1);

p = zeros(size(X, 1), num_labels);

h1 = sigmoid([ones(m, 1) X] * Theta1');
h2 = sigmoid([ones(m, 1) h1] * Theta2');


for j = 0:(number_of_nodes - 1)
	[dummy, v] = max(h2(:,(1 + j * (number_of_nodes + 1)):((j + 1) * (number_of_nodes + 1))), [], 2);
	for i = 1:m
		p(i, v(i) + j * (number_of_nodes + 1)) = 1;
	end
end


end
