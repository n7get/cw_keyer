def html_to_c_string(html_file):
    """Reads an HTML file and converts it to a C string."""
    with open(html_file, 'r') as file:
        lines = file.readlines()

    c_string = ""
    for line in lines:
        # Strip trailing whitespace and escape double quotes
        escaped_line = line.rstrip().replace('"', '\\"')
        c_string += f'"{escaped_line}\\n"\n'

    return c_string


def main():
    # Path to the HTML file
    html_file = "index.html"
    # Output C code
    c_code = html_to_c_string(html_file)

    print("Generated C code:")
    print(c_code)


if __name__ == "__main__":
    main()