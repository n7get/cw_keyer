import sys

def html_to_c_string(html_file):
    """Reads an HTML file and converts it to a C string."""
    with open(html_file, 'r') as file:
        lines = file.readlines()

    c_string = ""
    for line in lines:
        # Strip trailing whitespace and escape double quotes
        escaped_line = line.rstrip().replace('"', '\\"')
        c_string += f'"{escaped_line}\\n"\n'

    return c_string + ';'


def main():
    # Check if the input file is provided as a command-line argument
    if len(sys.argv) != 2:
        print("Usage: python3 conv_c.py <input_file>")
        sys.exit(1)

    # Get the input file name from the command-line argument
    html_file = sys.argv[1]

    try:
        # Convert the HTML file to a C string
        c_code = html_to_c_string(html_file)

        print(c_code)
    except FileNotFoundError:
        print(f"Error: File '{html_file}' not found.")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
