class NumberStore:
    #Stores numbers in input order and finds their 1-based index.
 
    def __init__(self, capacity):
        # Data initialization
        self.capacity = capacity
        self.numbers = []
 
    def add(self, value):
        # Data insertion
        if len(self.numbers) >= self.capacity:
            raise OverflowError("Store is full")
        self.numbers.append(value)
 
    def find(self, x):
        # Data search: 1-based index of first occurrence, or -1
        for i, value in enumerate(self.numbers, start=1):
            if value == x:
                return i
        return -1

def read_positive_int(prompt):
    while True:
        try:
            value = int(input(prompt))
            if value > 0:
                return value
        except ValueError:
            pass
        print("Please enter the total numbers that you want to enter: ")
 
def read_int(prompt):
    while True:
        try:
            return int(input(prompt))
        except ValueError:
            print("Please enter an integer.")
 
def main():
    n = read_positive_int("Please enter the total numbers that you want to enter: ")
    store = NumberStore(n)
 
    for i in range(1, n + 1):
        store.add(read_int(f"Enter number {i} of {n}: "))
 
    x = read_int("Enter X to search for: ")
    print(store.find(x))
 
if __name__ == "__main__":
    main()

#Output
#Please enter the total numbers that you want to enter: 8
#Enter number 1 of 8: 1
#Enter number 2 of 8: 2
#Enter number 3 of 8: 3
#Enter number 4 of 8: 4
#Enter number 5 of 8: 5
#Enter X to search for: 4
#4