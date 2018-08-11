# Image_Sonifier
An image sonifier - built for Berklee EPD-491 Capstone Project - Written in C

INSTRUCTIONS:

DISCLAIMER: This software requires that the user have Max MSP, and a current version of CSound installed.  Max MSP must be subscribed to, but CSound can be installed with a quick brew install (: 

Step 1:
  Open the maxpatch, "Image_Sonifier_readin", and hit the read message at the top of the screen.  This will allow you to read an image into the jitter matrix.  Once you have selected your image, you will not see it in the view windows that will appear.

Step 2:
  Click the write messages for each channel of the image you'd like to write out to a text file.  *NOTE* The Red, Green, and Blue files MUST, i repeat, **MUST** be named "red.txt", "green.txt", and "blue.txt", the sonifier will not work if these files are named otherwise.  
  
Step 3:
  Once all of the text files have been written to the Image Sonifier's folder, click the change color button, or drag the integer box near the top left to send the selected photo into the jitter matrix, and write the output of that jitter matrix to your R, G, and B text files. 
  
Step 4:
  Run the executable from the command line ("./betterbox"), enter a name for the csound file that will be created, and let the program run.
  
  
