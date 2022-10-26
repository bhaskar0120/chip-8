from sys import argv

def main():
    USAGE="Converts a textfile of hex commands seperated by newlines to ch8 file \nUSAGE: ./py tester.py [FILENAME]"
    if len(argv) < 2:
        print(USAGE)
        exit(1)
    else:
        try:
            with open(argv[1]) as readf:
                inst = [i.rsplit()[0] for i in readf.readlines()]
            with open("testfile.ch8","wb") as writef:
                for i in inst:
                    writef.write(int(i,16).to_bytes(2,'big'))
        except FileNotFoundError:
            print(f"{argv[1]} file not found")
            exit(1)

if __name__ == "__main__":
    main()
