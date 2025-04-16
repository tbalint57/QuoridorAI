#include <Eigen/Dense>
#include <vector>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <chrono>
#include <fstream>
#include <thread>
#include "board.cpp"
#include <string>
#include <iomanip>
#include <cmath>

// This file uses snake case, maybe I will change the other code in the future, but this is so much easier to read...
 
using namespace Eigen;
using namespace std;

using input_vector = Array<uint8_t, 142, 1>;

class Quoridor_GP {
    public:
    Quoridor_GP(){}

    Quoridor_GP(double sigma2, double lambda, vector<double> kernel_params)
        : sigma2_(sigma2), lambda_(lambda), kernel_params(kernel_params) {}


    void fit(const vector<input_vector>& X_train, const MatrixXd& Y_train) {
        X_train_ = X_train;
        MatrixXd K = compute_kernel_matrix(X_train_, sigma2_, lambda_);

        // Seems to be faster than K.inverse() ~2x
        LLT<MatrixXd> llt(K);
        K_inv_ = llt.solve(Y_train);
    }


    VectorXd predict(const input_vector& x_star) const {
        VectorXd k_star = compute_kernel_vector(x_star);
        VectorXd output = k_star.transpose() * K_inv_;

        // Eliminate illegal pawn moves
        uint8_t possiblePawnMoves[12] = {1, 2, 5, 6, 16, 24, 32, 40, 17, 21, 25, 29};
        for(int i = 0; i < 12; i++){
            uint8_t m = possiblePawnMoves[i];
            if (!((x_star(9) == m) || (x_star(10) == m) || (x_star(11) == m) || (x_star(12) == m) || (x_star(13) == m))) {
                output(m) = 0;
            }
        }

        return normalise_output(output);
    }


    void save(string filename) {
        ofstream out(filename, ios::binary);

        // Save hyperparametres
        out.write((char*) &sigma2_, sizeof(double));
        out.write((char*) &lambda_, sizeof(double));
        size_t kernel_param_size = kernel_params.size();
        out.write((char*) &kernel_param_size, sizeof(size_t));
        out.write((char*) kernel_params.data(), sizeof(double) * kernel_param_size);

        // Save X_train
        size_t num_inputs = X_train_.size();
        out.write((char*) &num_inputs, sizeof(size_t));
        for (auto& x : X_train_) {
            out.write((char*) x.data(), sizeof(uint8_t) * 142);
        }

        // Save K_inv_
        int rows = K_inv_.rows();
        int cols = K_inv_.cols();
        out.write((char*) &rows, sizeof(int));
        out.write((char*) &cols, sizeof(int));
        out.write((char*) K_inv_.data(), sizeof(double) * rows * cols);

        out.close();
    }


    static Quoridor_GP load(const string& filename) {
        ifstream in(filename, ios::binary);

        // Load hyperparametres
        double sigma2, lambda;
        in.read((char*) &sigma2, sizeof(double));
        in.read((char*) &lambda, sizeof(double));
        size_t kernel_param_size;
        in.read((char*) &kernel_param_size, sizeof(size_t));
        vector<double> kernel_params(kernel_param_size);
        in.read((char*) kernel_params.data(), sizeof(double) * kernel_param_size);
    
        // Initialise model
        Quoridor_GP model(sigma2, lambda, kernel_params);

        // Load X_train
        size_t num_inputs;
        in.read((char*) &num_inputs, sizeof(size_t));
        model.X_train_.resize(num_inputs);
        for (size_t i = 0; i < num_inputs; ++i) {
            in.read((char*) model.X_train_[i].data(), sizeof(uint8_t) * 142);
        }

        // Load K_inv_
        int rows, cols;
        in.read((char*) &rows, sizeof(int));
        in.read((char*) &cols, sizeof(int));
        model.K_inv_ = MatrixXd(rows, cols);
        in.read((char*) model.K_inv_.data(), sizeof(double) * rows * cols);

        in.close();
        return model;
    }


    void print_model_info() const {
        cout << fixed << setprecision(4);
        cout << "Sigma2: " << sigma2_
             << ", Lambda: " << lambda_
             << ", Kernel Params: [";
    
        for (size_t i = 0; i < kernel_params.size(); ++i) {
            cout << kernel_params[i];
            if (i < kernel_params.size() - 1) cout << ", ";
        }
        cout << "]" << endl;
    }

    private:

    double sigma2_;
    double lambda_;
    vector<double> kernel_params;
    vector<input_vector> X_train_;
    MatrixXd K_inv_;


    int wall_difference(const input_vector& x1, const input_vector& x2) const {
        int difference = 0;
        for(int i = 14; i < 142; i++){
            difference += x1(i) & x2(i);
        }
        return difference;
    }


    double custom_kernel(const input_vector& x1, const input_vector& x2) const {
        // Pawn placements
        double dist = abs(x1(0) - x2(0)) + abs(x1(1) - x2(1));

        // Wall difference
        dist += kernel_params[0] * wall_difference(x1, x2);

        // Players' wall count
        dist += kernel_params[1] * (abs(x1(2) - x2(2)) + abs(x1(3) - x2(3)));

        // Players' distance to goal
        dist += kernel_params[2] * (abs(x1(4) - x2(4)) + abs(x1(5) - x2(5)));

        // Distance between pawns
        dist += kernel_params[3] * (abs(x1(6) - x2(6)));

        // Players' number of available pawn movements
        dist += kernel_params[4] * (abs(x1(7) - x2(7)) + abs(x1(8) - x2(8)));

        return sigma2_ * exp(-(double) dist / lambda_);
    }


    MatrixXd compute_kernel_matrix(const vector<input_vector>& inputs, double sigma2, double lambda) const {
        int N = inputs.size();
        MatrixXd K(N, N);
        for (int i = 0; i < N; ++i) {
            for (int j = i; j < N; ++j) {
                double k = custom_kernel(inputs[i], inputs[j]);
                K(i, j) = K(j, i) = k;
            }
        }
        return K;
    }


    VectorXd compute_kernel_vector(const input_vector& x_star) const {
        int N = X_train_.size();
        VectorXd k_star(N);
        for (int i = 0; i < N; ++i) {
            k_star(i) = custom_kernel(x_star, X_train_[i]);
        }
        return k_star;
    }


    VectorXd normalise_output(const VectorXd& raw_output) const {
        VectorXd result = raw_output.array().max(0.0);
        double sum = result.sum();
        if (sum > 0.0){
            return result / sum;
        } 

        return VectorXd::Constant(result.size(), 1.0 / result.size());
    }
};


void readInSaveFile(Board* boards, int distributions[][256], size_t& size, string saveFileName){
    ifstream file(saveFileName, ios::in | ios::binary);

    char ch;
    while (file.get(ch)) {
        size_t board_size = ch - 1;
        char whitePawn, blackPawn, whiteWalls, blackWalls;

        file.get(whitePawn);
        file.get(blackPawn);
        file.get(whiteWalls);
        file.get(blackWalls);

        char WallsOnBoard[20];
        size_t numberOfWallsOnBoard = board_size - 4;

        for(int i = 0; i < numberOfWallsOnBoard; i++){
            file.get(WallsOnBoard[i]);
        }
        boards[size] = Board(whitePawn, blackPawn, whiteWalls, blackWalls, WallsOnBoard, numberOfWallsOnBoard);

        file.read((char*)distributions[size], 256 * sizeof(int));

        size++;
    }

    file.close();
}


tuple<vector<input_vector>, MatrixXd> load_dataset(const string& filename, bool player) {
    const size_t max_boards = 1000;
    Board* boards = new Board[max_boards];
    int (*distributions)[256] = new int[max_boards][256];
    size_t size = 0;

    readInSaveFile(boards, distributions, size, filename);

    vector<input_vector> X;
    MatrixXd Y(size, 256);

    for (size_t i = 0; i < size; ++i) {
        input_vector vec = boards[i].toInputVector(player);
        X.push_back(vec);

        for (int j = 0; j < 256; ++j) {
            Y(i, j) = (double) distributions[i][j];
        }
    }

    delete[] boards;
    delete[] distributions;

    return {X, Y};
}


Quoridor_GP hyperparameter_search(const string& train_file, const string& val_file, bool player, int num_trials = 100) {
    auto [X_train, Y_train] = load_dataset(train_file, player);
    auto [X_val, Y_val] = load_dataset(val_file, player);

    random_device rd;
    mt19937 gen(rd());

    uniform_real_distribution<double> dist_sigma2(5.0, 50.0);
    uniform_real_distribution<double> dist_lambda(5.0, 20.0);
    uniform_real_distribution<double> dist_kernel_param(0.5, 2.0);

    const int kernel_param_count = 5; // Number of kernel parameters

    double best_loss = 1e9;
    double best_sigma2 = 0.0;
    double best_lambda = 0.0;
    vector<double> best_kernel_params;

    for (int trial = 0; trial < num_trials; ++trial) {
        double sigma2 = dist_sigma2(gen);
        double lambda = dist_lambda(gen);
        vector<double> kernel_params(kernel_param_count);
        for (int i = 0; i < kernel_param_count; ++i) {
            kernel_params[i] = dist_kernel_param(gen);
        }

        Quoridor_GP model(sigma2, lambda, kernel_params);
        model.fit(X_train, Y_train);

        double total_loss = 0.0;

        for (size_t i = 0; i < X_val.size(); ++i) {
            VectorXd pred = model.predict(X_val[i]);

            VectorXd actual = Y_val.row(i);
            double sum = actual.sum();
            if (sum > 0.0) {
                actual /= sum;
            } else {
                actual = VectorXd::Constant(256, 1.0 / 256);
            }

            total_loss += (pred - actual).squaredNorm();
        }

        double avg_loss = total_loss / X_val.size();

        if (avg_loss < best_loss) {
            best_loss = avg_loss;
            best_sigma2 = sigma2;
            best_lambda = lambda;
            best_kernel_params = kernel_params;
        }
    }

    Quoridor_GP best_model(best_sigma2, best_lambda, best_kernel_params);
    best_model.fit(X_train, Y_train);
    return best_model;
}


void train_and_save_gp_model(int i) {
    string white_train_file = "datasets/datasetWhite" + to_string(i);
    string black_train_file = "datasets/datasetBlack" + to_string(i);

    string white_model_file = "GPmodels/whiteModel" + to_string(i);
    string black_model_file = "GPmodels/blackModel" + to_string(i);
    string small_white_model_file = "GPmodels/whiteModelSmall" + to_string(i);
    string small_black_model_file = "GPmodels/blackModelSmall" + to_string(i);

    Quoridor_GP white_model = hyperparameter_search(white_train_file + ".train", white_train_file + ".val", true);
    white_model.save(white_model_file);

    Quoridor_GP black_model = hyperparameter_search(black_train_file + ".train", black_train_file + ".val", false);
    black_model.save(black_model_file);

    Quoridor_GP small_white_model = hyperparameter_search(white_train_file + ".val", white_train_file + ".train", true);
    small_white_model.save(small_white_model_file);

    Quoridor_GP small_black_model = hyperparameter_search(black_train_file + ".val", black_train_file + ".train", false);
    small_black_model.save(small_black_model_file);
}


void pre_train_models(){
    vector<thread> threads;

    for (int i = 0; i <= 20; ++i) {
        threads.emplace_back(train_and_save_gp_model, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    cout << "pre-training finished" << endl;
}