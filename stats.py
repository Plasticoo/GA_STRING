import subprocess
import string
import random

from numpy import arange

test_strings = []


def gen_rand_strings(chars=string.ascii_uppercase +
                     string.digits +
                     string.ascii_lowercase):
    test_strings.append(''.join(random.choice(chars) for _ in range(6)))
    test_strings.append(''.join(random.choice(chars) for _ in range(8)))
    test_strings.append(''.join(random.choice(chars) for _ in range(16)))
    test_strings.append(''.join(random.choice(chars) for _ in range(32)))

    for _string in test_strings:
        print('[INFO] - Generated string: {}'.format(_string))

    print('')


def main():
    gen_rand_strings()

    for _string in test_strings:
        for population in range(1000, 100000, 1000):
            for gens in range(1000, 50000, 1000):
                for mutation in arange(0.05, 1.00, 0.05):
                    for crossover in arange(0.05, 1.00, 0.05):
                        res = subprocess.Popen(["./program",
                                                "-c",
                                                str(crossover),
                                                "-e",
                                                "-g",
                                                str(gens),
                                                "-m",
                                                str(mutation),
                                                "-p",
                                                str(population),
                                                "-o",
                                                "-w",
                                                _string])

                        res.wait()

                        if res.returncode != 0:
                            print('[ERROR] - Main program returned error.')
                            exit(1)


main()
