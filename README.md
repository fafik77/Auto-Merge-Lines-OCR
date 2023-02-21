# Auto Merge Lines OCR
Smart merge lines from multiple text files (intended for <b>OCR</b>) into one<br>
In case some parts of text(header, footer) appear in multiple files only one of those wil be added

<li>can sort files by Name,Date</li>
<li>can gather files from subdirs</li>
<li>can use wildcard to specify input files</li>
<li>should work with UTF16 paths</li>

<br>Command Line Arguments:<br>
  [1]: (string) input files wildcard like "\*.txt"<br>
  [2]: (string) output file name<br>
  [\* options]<br>
  <b>options \*:</b><br>
   /S with subdirectories (files order will be automatic)<br>
   /O: SortOrder:<br>
   N  by Name(alphabetic)  D  by Date/time (oldest first)<br>
   <b>\-</b> Prefix to reverse order<br>
   /P Pause to confirm file order<br>
   /n-	Word is separated into 2 lines ending with '-'<br>
   /nn	Fix Text is FitToSpace followed by empty line<br>
   /fix$	Fix common OCR Paragraph mistakes [$85]<br>
   /M:[50-100]	Match minimum n percent of Line to satisfy (def. 100)<br>
   /CQ:[0-4]	Consecutive matches forLines (i.e. the icon's red line) (def. 0)<br>
<br>Example: "files\\*.txt" MergedLines.txt /O:N<br>
