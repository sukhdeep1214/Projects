def find_number_index(x, numbers):
  #Search for X in N and output index or -1
  if x in numbers:
    return numbers.index(x)
  else:
    return -1
        
if __name__ == "__main__":
  #Ask for total number of Elements that User wants to enter
  n = int(input("Enter the total number that you want to enter N:"))
    
  #Read the N numbers one by one and store them in a list
  numbers = []
  for i in range(n):
    num = int(input(f"Enter number {i + 1}: "))
    numbers.append(num)
    
  #Ask the user integer X that user wants to search 
  x = int(input("Enter the integer X to search for: "))
    
  result = find_number_index(x, numbers)
  print(result)
 
#Output
#Enter the total number that you want to enter N:5
#Enter number 1: 1
#Enter number 2: 2
#Enter number 3: 3
#Enter number 4: 4
#Enter number 5: 5
#Enter the integer X to search for: 4
#3