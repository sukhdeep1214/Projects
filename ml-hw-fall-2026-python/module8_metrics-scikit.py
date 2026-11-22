import numpy as np
from sklearn.metrics import precision_score, recall_score

def read_positive_int(prompt):
    #Keep asking until the user enters a positive integer.
    while True:
        try:
            value = int(input(prompt).strip())
            if value > 0:
                return value
            print("Please enter a positive integer (N > 0).")
        except ValueError:
            print("Invalid input. Please enter a positive integer.")

def read_binary(prompt):
    #Keep asking until the user enters 0 or 1.
    while True:
        try:
            value = int(input(prompt).strip())
            if value in (0, 1):
                return value
            print("Value must be 0 or 1.")
        except ValueError:
            print("Invalid input. Please enter 0 or 1.")

def main():
    n = read_positive_int("Enter N (number of points, positive integer): ")

    # Data initialization (NumPy): row i holds (x_i, y_i)
    points = np.zeros((n, 2), dtype=int)

    # Data insertion (NumPy)
    for i in range(n):
        print(f"Point {i + 1} of {n}:")
        points[i, 0] = read_binary("  x (true class, 0 or 1): ")
        points[i, 1] = read_binary("  y (predicted class, 0 or 1): ")

    y_true = points[:, 0]
    y_pred = points[:, 1]

    # Computation (scikit-learn); zero_division=0 avoids errors/warnings when
    # there are no predicted positives (precision) or no actual positives (recall)
    precision = precision_score(y_true, y_pred, zero_division=0)
    recall = recall_score(y_true, y_pred, zero_division=0)

    print(f"\nPrecision: {precision:.4f}")
    print(f"Recall:    {recall:.4f}")

if __name__ == "__main__":
    main()