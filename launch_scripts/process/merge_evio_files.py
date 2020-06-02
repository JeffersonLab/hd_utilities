import os, sys
from optparse import OptionParser


def get_file_size(filename):
    statinfo = os.stat(filename)
    return statinfo.st_size


def merge_files(merged_filename, file_list, blocksize=10485760L, verbose=True):
    """
    blocksize = 10M  (number in bytes)
    """
    with open(merged_filename, 'wb') as outf:
        for infilename in file_list:
            try:
                with open(infilename, 'rb') as inf:
                    if verbose:
                        print "processing "+infilename+" ..."
                    length_to_read = get_file_size(infilename)
                    while(length_to_read > blocksize):
                        bytes = inf.read(blocksize)
                        outf.write(bytes)
                        if verbose:
                            print "bytes to read = %d"%length_to_read
                            #print "bytes written = %d"%numwritten
                            #print "bytes written "+str(numwritten)
                        length_to_read -= blocksize
                    # don't read in trailing 8 words
                    if infilename == file_list[-1]:
                        bytes = inf.read(length_to_read)
                    else:
                        # strip out EOF blocks for all files but the last one
                        bytes = inf.read(length_to_read - 8*4)
                        #bytes = inf.read(length_to_read)
                    outf.write(bytes)
                    if verbose:
                            print "bytes to read = %d"%length_to_read
                            #print "bytes written = "+str(numwritten)
            except OSError:
                print "Could not open " + infilename


if __name__ == "__main__":
    # read in command line args
    parser = OptionParser(usage = "merge_evio_files.py merged_file file1 [file2 ...]")
    parser.add_option("-D","--output_dir", dest="output_dir",
                      help="Directory where output files will be stored")

    (options, args) = parser.parse_args(sys.argv)
    if(len(args) < 2):
        parser.print_help()
        sys.exit(0)

    # defaults
    #merged_filename = "merged.evio"
    merged_filename = "/scratch/gxproj5/merged.evio"
    
    merge_files(merged_filename, args[1:])
