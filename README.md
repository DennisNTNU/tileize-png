# tileize-png
Small program to split a png file into 256x256 tiles at multiple zoom levels

### Instructions

#### Building
```
git clone --recursre-submodules https://github.com/DennisNTNU/tileize-png
cd tileize-png
mkdir build
cd build
cmake ..
make
```

#### Using

```
./tileize-png <path to png image file>
```

This will generate a folder called `tiles` at the same place as the input png file. Inside `tiles` are subfolders `1`, `2`, etc, containing the 256x256 tiles as png files. The numbers indicate zoom levels where `1` is most zoomed out.
