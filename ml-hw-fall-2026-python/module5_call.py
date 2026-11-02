from module5_mod import NumberStore
 
def read_positive_int(prompt):
    while True:
        try:
            value = int(input(prompt))
            if value > 0:
                return value
        except ValueError:
            pass
        print("Please enter a positive integer.")
 
def read_int(prompt):
    while True:
        try:
            return int(input(prompt))
        except ValueError:
            print("Please enter an integer.")
 
def main():
    n = read_positive_int("Enter N (positive integer): ")
    store = NumberStore(n)
 
    for i in range(1, n + 1):
        store.add(read_int(f"Enter number {i} of {n}: "))
 
    x = read_int("Enter X to search for: ")
    print(store.find(x))
 
if __name__ == "__main__":
    main()