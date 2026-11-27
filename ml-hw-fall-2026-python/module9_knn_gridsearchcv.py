import numpy as np
from sklearn.metrics import accuracy_score
from sklearn.model_selection import GridSearchCV
from sklearn.neighbors import KNeighborsClassifier


def read_positive_int(prompt: str) -> int:
    while True:
        try:
            value = int(input(prompt))
            if value > 0:
                return value
            print("Please enter a positive integer.")
        except ValueError:
            print("Invalid input. Please enter a positive integer.")


def read_dataset(num_samples: int, set_name: str):
    # Initialize NumPy arrays for features (X) and labels (y)
    X = np.empty((num_samples, 1), dtype=np.float64)
    y = np.empty(num_samples, dtype=np.int64)

    print(f"Enter {num_samples} (x, y) pairs for {set_name}:")
    for i in range(num_samples):
        while True:
            try:
                x_val = float(input(f"Pair {i + 1} - Enter x (real number): "))
                break
            except ValueError:
                print("Invalid input. Please enter a real number for x.")

        while True:
            try:
                y_val = int(
                    input(f"Pair {i + 1} - Enter y (non-negative integer): ")
                )
                if y_val >= 0:
                    break
                print("Please enter a non-negative integer for y.")
            except ValueError:
                print(
                    "Invalid input. Please enter a non-negative integer for y."
                )

        # Insert values into NumPy arrays
        X[i, 0] = x_val
        y[i] = y_val

    return X, y


def main():
    # 1. Read training set TrainS
    N = read_positive_int("Enter N (number of training samples): ")
    X_train, y_train = read_dataset(N, "TrainS")

    # 2. Read test set TestS
    M = read_positive_int("Enter M (number of test samples): ")
    X_test, y_test = read_dataset(M, "TestS")

    # 3. Configure GridSearchCV for kNN Classification (1 <= k <= 10)
    if N == 1:
        # Fallback when N=1 since cross-validation requires at least 2 samples
        best_k = 1
        best_model = KNeighborsClassifier(n_neighbors=best_k)
        best_model.fit(X_train, y_train)
    else:
        cv_folds = min(5, N)
        # Ensure k does not exceed the smallest training fold size during CV
        min_train_fold_size = N - (N + cv_folds - 1) // cv_folds
        max_k = min(10, max(1, min_train_fold_size))

        param_grid = {"n_neighbors": list(range(1, max_k + 1))}
        knn = KNeighborsClassifier()

        grid_search = GridSearchCV(
            estimator=knn, param_grid=param_grid, cv=cv_folds, scoring="accuracy"
        )
        grid_search.fit(X_train, y_train)

        best_k = grid_search.best_params_["n_neighbors"]
        best_model = grid_search.best_estimator_

    # 4. Evaluate on TestS
    y_pred = best_model.predict(X_test)
    test_accuracy = accuracy_score(y_test, y_pred)

    # 5. Output results
    print(f"\nBest k: {best_k}")
    print(f"Test accuracy: {test_accuracy:.4f}")


if __name__ == "__main__":
    main()
