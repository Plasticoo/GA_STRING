import subprocess

test_strings = [""]

res = subprocess.run(["ls", "-l"], check=True)

print(res.returncode)
