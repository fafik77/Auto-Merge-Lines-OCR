# Auto Merge Lines OCR
Smart merge lines from multiple text files (intended for <b>OCR</b>) into one

<li>can sort files by Name,Date</li>
<li>can gather files from subdirs</li>
<li>can use wildcard to specify input files</li>
<li>should work with UTF16 paths</li>

<br>Command Line Arguments:<br>
  [1]: (string) input files wildcard like "\*.txt"<br>
  [2]: (string) output file name<br>
  [\* options]<br>
  options \*:<br>
   /S with subdirectories (files order will be automatic)<br>
   /O: SortOrder:<br>
   N  by Name(alphabetic)  D  by Date/time (oldest first)<br>
   <b>\-</b> Prefix to reverse order<br>
   /P Pause to confirm file order<br>
Example: "files\\*.txt" MergedLines.txt /O:N<br>
