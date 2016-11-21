clear ; close all; clc

number_of_nodes = 12;
dimension = 2;

input_layer_size  = number_of_nodes * dimension * 2;
hidden_layer_size = 440;
num_labels = number_of_nodes * (number_of_nodes + 1);

fprintf('Loading train.txt\n');
fflush(stdout);
load('train.txt');

tr_size = 10000;
val_size = 500;
test_size = 500;
% TODO
% Set (training set, validation set, test set) with percentages.

% Randomly select 'tr_size' data points for training set
size(XY)
sel = randperm(size(XY, 1));
sel_tr = sel(1:tr_size);
sel_val = sel((tr_size + 1):(tr_size + val_size));
sel_test = sel((tr_size + val_size + 1):end);

X = reshape(XY(sel_tr, 1:input_layer_size), tr_size, input_layer_size);
y = reshape(XY(sel_tr, (input_layer_size + 1):end), tr_size, num_labels);

X_val = reshape(XY(sel_val, 1:input_layer_size), val_size, input_layer_size);
y_val = reshape(XY(sel_val, (input_layer_size + 1):end), val_size, num_labels);

X_test = reshape(XY(sel_test, 1:input_layer_size), test_size, input_layer_size);
y_test = reshape(XY(sel_test, (input_layer_size + 1):end), test_size, num_labels);

fprintf('Loading finished for X, y, X_val, y_val, X_test, y_test. Press enter to continue.\n');
pause;

fprintf('\nInitializing Neural Network Parameters ...\n')
fflush(stdout);

initial_Theta1 = randInitializeWeights(input_layer_size, hidden_layer_size);
initial_Theta2 = randInitializeWeights(hidden_layer_size, num_labels);

% Unroll parameters
initial_nn_params = [initial_Theta1(:) ; initial_Theta2(:)];

fprintf('\nTraining Neural Network... \n')
fflush(stdout);

options = optimset('MaxIter', 100);

lambda = 1;

% Create "short hand" for the cost function to be minimized
costFunction = @(p) nnCostFunction(p, ...
                                   input_layer_size, ...
                                   hidden_layer_size, ...
                                   num_labels, X, y, lambda);

% Now, costFunction is a function that takes in only one argument (the
% neural network parameters)
[nn_params, cost] = fmincg(costFunction, initial_nn_params, options);

Theta1 = reshape(nn_params(1:hidden_layer_size * (input_layer_size + 1)), ...
                 hidden_layer_size, (input_layer_size + 1));

Theta2 = reshape(nn_params(1 + (hidden_layer_size * (input_layer_size + 1)):end), ...
                 num_labels, (hidden_layer_size + 1));

fprintf('Program paused. Press enter to continue.\n');
pause;

pred_tr = predict(Theta1, Theta2, number_of_nodes, X);

fprintf('\nTraining Set Accuracy: %f\n', mean(double((sum(pred_tr == y, 2) / num_labels) == 1)) * 100);

pred_val = predict(Theta1, Theta2, number_of_nodes, X_val);

fprintf('\nValidation Set Accuracy: %f\n', mean(double((sum(pred_val == y_val, 2) / num_labels) == 1)) * 100);

X_opt = XY(sel_val,:);
save X_opt.txt X_opt
save X_pred.txt pred_val









