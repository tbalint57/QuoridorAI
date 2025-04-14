#include <Eigen/Dense>
#include <vector>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <chrono>
#include <fstream>
#include "dataGeneration.cpp"

// This file uses snake case, maybe I will change the other code in the future, but this is so much easier to read...
 
using namespace Eigen;
using namespace std;

using input_vector = Array<uint8_t, 160, 1>;


class Quoridor_GP {
    public:

    Quoridor_GP(double sigma2, double lambda)
        : sigma2_(sigma2), lambda_(lambda) {}


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
        return normalise_output(output);
    }


    void save(string filename) {
        ofstream out(filename, ios::binary);

        // Save hyperparametres
        out.write((char*) &sigma2_, sizeof(double));
        out.write((char*) &lambda_, sizeof(double));

        // Save X_train
        size_t num_inputs = X_train_.size();
        out.write((char*) &num_inputs, sizeof(size_t));
        for (auto& x : X_train_) {
            out.write((char*) x.data(), sizeof(uint8_t) * 160);
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
        Quoridor_GP model(sigma2, lambda);

        // Load X_train
        size_t num_inputs;
        in.read((char*) &num_inputs, sizeof(size_t));
        model.X_train_.resize(num_inputs);
        for (size_t i = 0; i < num_inputs; ++i) {
            in.read((char*) model.X_train_[i].data(), sizeof(uint8_t) * 160);
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

    private:

    double sigma2_;
    double lambda_;
    vector<input_vector> X_train_;
    MatrixXd K_inv_;


    int hamming_distance(const input_vector& x1, const input_vector& x2) const {
        return (x1 != x2).count();
    }


    double hamming_kernel(const input_vector& x1, const input_vector& x2) const {
        int dist = hamming_distance(x1, x2);
        return sigma2_ * exp(-(double) dist / lambda_);
    }


    MatrixXd compute_kernel_matrix(const vector<input_vector>& inputs, double sigma2, double lambda) const {
        int N = inputs.size();
        MatrixXd K(N, N);
        for (int i = 0; i < N; ++i) {
            for (int j = i; j < N; ++j) {
                double k = hamming_kernel(inputs[i], inputs[j]);
                K(i, j) = K(j, i) = k;
            }
        }
        return K;
    }


    VectorXd compute_kernel_vector(const input_vector& x_star) const {
        int N = X_train_.size();
        VectorXd k_star(N);
        for (int i = 0; i < N; ++i) {
            k_star(i) = hamming_kernel(x_star, X_train_[i]);
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


tuple<vector<input_vector>, MatrixXd> load_dataset(const string& filename) {
    const size_t max_boards = 1000;
    Board* boards = new Board[max_boards];
    int (*distributions)[256] = new int[max_boards][256];
    size_t size = 0;

    readInSaveFile(boards, distributions, size, filename);

    vector<input_vector> X;
    MatrixXd Y(size, 256);

    for (size_t i = 0; i < size; ++i) {
        input_vector vec = boards[i].toInputVector();
        X.push_back(vec);

        for (int j = 0; j < 256; ++j) {
            Y(i, j) = (double) distributions[i][j];
        }
    }

    delete[] boards;
    delete[] distributions;

    return {X, Y};
}


Quoridor_GP hyperparameter_search(string train_file, string val_file) {
    // Load train and validation sets
    auto [X_train, Y_train] = load_dataset(train_file);
    auto [X_val, Y_val] = load_dataset(val_file);

    // Grid search ranges
    vector<double> sigma2_values = {1.0, 0.5, 0.2, 0.1, 0.05, 0.02, 0.01};
    vector<double> lambda_values = {1.0, 0.5, 0.2, 0.1, 0.05, 0.02, 0.01};

    // Loss is at max 4
    double best_loss = 1000000.0;
    double best_sigma2 = 0.0;
    double best_lambda = 0.0;

    for (double sigma2 : sigma2_values) {
        for (double lambda : lambda_values) {
            Quoridor_GP model(sigma2, lambda);
            model.fit(X_train, Y_train);

            double total_loss = 0.0;

            for (size_t i = 0; i < X_val.size(); ++i) {
                VectorXd pred = model.predict(X_val[i]);
                VectorXd actual = Y_val.row(i) / Y_val.row(i).sum();
                double loss = (pred - actual).squaredNorm();
                total_loss += loss;
            }

            double avg_loss = total_loss / X_val.size();

            if (avg_loss < best_loss) {
                best_loss = avg_loss;
                best_sigma2 = sigma2;
                best_lambda = lambda;
                cout << "New best (sigma2=" << sigma2 << ", lambda=" << lambda << ") with loss = " << avg_loss << endl;
            }
        }
    }

    Quoridor_GP best_model(best_sigma2, best_lambda);
    best_model.fit(X_train, Y_train);
    return best_model;
}

 
int main() {
    srand(time(NULL));
    for(int i = 0; i < 21; i++){
        string whiteTrain = "datasets/datasetWhite" + to_string(i) + ".train";
        string whiteVal = "datasets/datasetWhite" + to_string(i) + ".val";
        string whiteSave = "GPmodels/whiteModel" + to_string(i); 
        hyperparameter_search(whiteTrain, whiteVal).save(whiteSave);

        string blackTrain = "datasets/datasetBlack" + to_string(i) + ".train";
        string blackVal = "datasets/datasetBlack" + to_string(i) + ".val";
        string blackSave = "GPmodels/blackModel" + to_string(i); 
        hyperparameter_search(blackTrain, blackVal).save(blackSave);
    }
}