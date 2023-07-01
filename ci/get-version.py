import sys
import yaml

if len(sys.argv) < 2:
    print("invalid argv", file=sys.stderr)
    exit(2)

manifest_file = sys.argv[1]
try:
    with open(manifest_file) as manifest:
        dat = manifest.read()
except:
    print("idf_component file notfound or cannot access.", file=sys.stderr)
    exit(2)

value = yaml.load(dat, Loader=yaml.FullLoader)

if type(value) is dict and 'version' in value.keys():
    print(value['version'])
    exit(0)
else:
    print("version notfound.", file=sys.stderr)
    exit(2)
