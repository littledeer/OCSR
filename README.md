OCSR
====

optical chemical structure recognition
v0.0.1

============
INTRODUCTION
============
This is a optical chemical struture recognition software. It can be used to recognize the chemicial struture image. The output is a molecular structure encoded in JME format.


=======
INSTALL
=======
You need to create a vistual studio project and copy the ChemIdentify, opencv248.props, potrace_1-11.props and ocrs.props to the root directory of the project. Add Property Sheets "opencv248.props, potrace_1-11.props and ocrs.props" to the vistual studio project(Debug).

Add system environment variables as follows:
GOCR_HOME = X:\XXX\XXX\ChemIdentify\gocr
OCRAD_HOME = X:\XXX\XXX\ChemIdentify\ocrad
OPENCV_HOME = X:\XXX\XXX\ChemIdentify\opencv
POTRACE_HOME = X:\XXX\XXX\ChemIdentify\potrace
TBB_HOME = X:\XXX\XXX\ChemIdentify\tbb
TESSDATA_PREFIX = X:\XXX\XXX\ChemIdentify\tesseract
Add %TBB_HOME%\bin\ia32\vc10;%OPENCV_HOME%\x86vc10\x86\vc10\bin;%POTRACE_HOME%\bin;%TESSDATA_PREFIX%\bin to "Path" environment variable

Now you can compile it!


==========
HOW TO USE
==========
Run the cmd.exe and enter the directory where the ChemIdentify.exe in. Input "ChemIdentify ImagePath"(e.g. "ChemIdentify c:/example.jpg"), then you can get a molecular structure encoded in JME format. The image formats that can be recognized including jpg, jpeg, bmp, png, pbm, pgm, ppm, tiff, tif. The result can be shown by JSME. JSME is a conversion of the JME applet into JavaScript and JME is a Java applet for creating and editing molecules. They are written by Peter Ertl.
JSME Homepage: http://ns1.peter-ertl.com/jsme/index.html
I also use JSME to show the recognition results online. You can find it at http://search.ipa361.com/Patent/RecognizeChemStru/recog_method