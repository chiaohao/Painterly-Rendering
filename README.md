# Painterly-Rendering

Compile
---------
g++ \`pkg-config opencv --cflags --libs\` painterly_rendering.cpp -o painterly_rendering -lopencv_imgcodecs -lopencv_core -lopencv_highgui -lopencv_imgproc

Run
---------
./painterly_rendering [origin picture] [brush size]