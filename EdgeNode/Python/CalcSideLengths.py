import argparse

def calculate_possible_third_side_length(a, b):
    # Check if a and b are positive
    if a <= 0 or b <= 0:
        return "Side lengths must be positive."

    # Calculate the possible range of values for the third side length, c
    min_c = abs(a - b) + 1  # Minimum possible value of c
    max_c = a + b - 1  # Maximum possible value of c


    return f"The possible range of values for the third side length (c) is [{min_c}, {max_c}]"


if __name__ == "__main__":
    # Parse command-line arguments
    parser = argparse.ArgumentParser(description="Calculate the possible range of values for the third side length of a triangle.")
    parser.add_argument("--a", type=float, default=4.0, help="Length of side a (default: 4)")
    parser.add_argument("b", type=float, help="Length of side b")
    args = parser.parse_args()

    # Calculate the possible range of values for the third side length
    result = calculate_possible_third_side_length(args.a, args.b)
    print("first side (a) is ",args.a, "second side (b) is ",args.b)
    print(result)
