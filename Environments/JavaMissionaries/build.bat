del /S /Q *.class 
javac -classpath .;..\..\SoarLibrary\bin\swt.jar;..\..\SoarLibrary\bin\sml.jar -sourcepath src src\edu\umich\mac\MissionariesAndCannibals.java
jar cfm mac.jar JarManifest -C src .
IF NOT "%1"=="--nopause" pause