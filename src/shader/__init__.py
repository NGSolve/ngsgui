import os, sys

# no idea if that is possible in a nicer way... 
ngsolve_path = os.path.abspath(sys.argv[0]).split()[0]
location = os.path.abspath(os.path.join(ngsolve_path, ".." , "..", "share", "ngsgui", "shader"))
