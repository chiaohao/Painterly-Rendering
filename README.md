# Painterly-Rendering

Description
---------
Applying Hertzmann's Panterly Rendering Algorithm with OpenCV 2.4.1 and C++.

Compile
---------
g++ \`pkg-config opencv --cflags --libs\` [cpp file name] -o [exec file name] -lopencv_imgcodecs -lopencv_core -lopencv_highgui -lopencv_imgproc

Run
---------
./painterly_rendering [origin picture] [layer count]