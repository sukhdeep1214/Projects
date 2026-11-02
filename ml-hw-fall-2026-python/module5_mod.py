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