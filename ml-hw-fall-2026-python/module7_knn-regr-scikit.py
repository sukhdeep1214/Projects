import numpy as np
from sklearn.neighbors import KNeighborsRegressor

def main():
    # 1. Read N (number of points)
    try:
        N = int(input("Enter N (positive integer - number of points): "))
        if N <= 0:
            print("Error: N must be a positive integer.")
            return
    except ValueError:
        print("Error: Invalid input. N must be an integer.")
        return

    # 2. Read k (number of neighbors)
    try:
        k = int(input("Enter k (positive integer - number of neighbors): "))
        if k <= 0:
            print("Error: k must be a positive integer.")
            return
    except ValueError:
        print("Error: Invalid input. k must be an integer.")
        return

    # 3. Read N (x, y) points
    print(f"Please provide {N} points:")
    points = np.zeros((N, 2))  # Initialize Numpy array for storage

    for i in range(N):
        try:
            x = float(input(f"Enter x value for point {i+1}: "))
            y = float(input(f"Enter y value for point {i+1}: "))
            points[i] = [x, y]
        except ValueError:
            print("Error: Invalid input. X and Y must be real numbers.")
            return

    # Separate features (X_train) and labels (y_train)
    X_train = points[:, 0].reshape(-1, 1)
    y_train = points[:, 1]

    # Calculate and output the variance of labels in the training dataset
    variance_y = np.var(y_train)
    print(f"\nVariance of labels (Y) in the training dataset: {variance_y:.4f}")

    # 4. Check if k <= N, otherwise throw an error
    if k > N:
        print(
            f"Error: k ({k}) cannot be greater than the number of available points N ({N})."
        )
        return

    # 5. Read target X for prediction
    try:
        X_predict = float(input("\nEnter X value to predict Y: "))
    except ValueError:
        print("Error: Invalid input. X must be a real number.")
        return

    # 6. Computation using Scikit-learn
    knn = KNeighborsRegressor(n_neighbors=k)
    knn.fit(X_train, y_train)

    # Reshape input for prediction as scikit-learn expects 2D array
    X_predict_arr = np.array([[X_predict]])
    y_pred = knn.predict(X_predict_arr)

    print(f"The predicted Y value for X = {X_predict} is: {y_pred[0]}")

if __name__ == "__main__":
    main()
