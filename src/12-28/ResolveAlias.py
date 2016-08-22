#! /usr/bin/python
# ResolveAlias.py
   
import sys
import Carbon.File
   
def main():
   
    if len(sys.argv) != 2:
        sys.stderr.write("usage: ResolveAlias <alias path>\n")
        return 1
   
    try:
        fsspec, isfolder, aliased = \
            Carbon.File.ResolveAliasFile(sys.argv[1], 0)
    except:
        raise "No such file or directory."
   
    print fsspec.as_pathname()
   
    return 0
   
if __name__ == "__main__":
    sys.exit(main())
