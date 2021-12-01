# get_file_time.py

This estimates the starting date and time for the evio file specified, using DCODAROCInfo stored in the supplied file and the first file for the same run, if that is available, or RCDB if it is not. 

A filename can be provided for concise output (optional).

Usage: python get_file_time.py <evio_file> [<output_file>]

```sh
python get_file_time.py /cache/halld/RunPeriod-2021-08/rawdata/Run081717/hd_rawdata_081717_010.evio time_81717_010.txt

```
