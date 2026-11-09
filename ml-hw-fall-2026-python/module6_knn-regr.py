import numpy as np

class KNNRegression:
    def __init__(self, k):
        # Initializes the k-NN Regression model with the number of neighbors.
        
        self.k = k
        self.points = None

    def fit(self, points):
        #Stores the data points. Expects a NumPy array of shape (N, 2).
        
        self.points = np.array(points, dtype=float)

    def predict(self, target_x):
        #Predicts the Y value for a given target X using k-NN regression.
        
        if self.points is None or len(self.points) == 0:
            raise ValueError("No data points available to make a prediction.")
        
        # Extract X and Y columns
        x_vals = self.points[:, 0]
        y_vals = self.points[:, 1]
        
        # Calculate absolute distances from target_x to all x values
        distances = np.abs(x_vals - target_x)
        
        # Get the indices of the k smallest distances
        # np.argsort returns indices that would sort the array
        nearest_indices = np.argsort(distances)[:self.k]
        
        # Calculate and return the mean of the Y values of the k nearest neighbors
        return np.mean(y_vals[nearest_indices])

def get_positive_integer(prompt):
    
    #Helper function to safely get a positive integer from the user.
    
    while True:
        try:
            val = int(input(prompt))
            if val <= 0:
                print("Error: Please enter a positive integer greater than 0.")
                continue
            return val
        except ValueError:
            print("Error: Invalid input. Please enter a valid integer.")

def get_real_number(prompt):
    
    #Helper function to safely get a real number (float) from the user.
    
    while True:
        try:
            return float(input(prompt))
        except ValueError:
            print("Error: Invalid input. Please enter a valid real number.")

def main():
    print("--- k-NN Regression ---")
    
    # Read input N (positive integer)
    N = get_positive_integer("Enter the number of points (N): ")
    
    # Read input k (positive integer)
    k = get_positive_integer("Enter the number of neighbors (k): ")
    
    # Check if k is greater than N early or let it proceed to data entry
    if k > N:
        print(f"Error: k ({k}) cannot be greater than the number of points N ({N}).")
        return

    # Read N (x, y) points one by one
    points_list = []
    print(f"\nPlease provide {N} points:")
    for i in range(N):
        print(f"Point {i + 1}:")
        x = get_real_number("  Enter x value: ")
        y = get_real_number("  Enter y value: ")
        points_list.append([x, y])
    
    # Initialize and fit the model using the OOP approach
    knn_model = KNNRegression(k=k)
    knn_model.fit(points_list)
    
    # Ask for target X input to predict
    print("\n--- Prediction ---")
    target_x = get_real_number("Enter the X value to predict Y for: ")
    
    # Calculate and output the result
    predicted_y = knn_model.predict(target_x)
    print(f"\nThe predicted Y value using {k}-NN Regression is: {predicted_y:.4f}")

if __name__ == "__main__":
    main()

#Output
#--- k-NN Regression ---
#Enter the number of points (N): 4
#Enter the number of neighbors (k): 1

#Please provide 4 points:
#Point 1:
#  Enter x value: 1
#  Enter y value: 2
#Point 2:
#  Enter x value: 1
#  Enter y value: 3
#Point 3:
#  Enter x value: 2
#  Enter y value: 4
#Point 4:
#  Enter x value: 2
#  Enter y value: 5

#--- Prediction ---
#Enter the X value to predict Y for: 3

#The predicted Y value using 1-NN Regression is: 5.0000