while True:
    r = float(input())
    x = (r * 2) + 1
    total_bytes = (x**5) * 3
    
    mb = total_bytes / (1024**2)
    gb = total_bytes / (1024**3)
    
    print("mb", mb)
    print("gb", gb)