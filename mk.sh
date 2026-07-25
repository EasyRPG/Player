cd /Users/wys/Documents/leasy
rm -rf build
cmake -B build \
  -DICU_ROOT=$(brew --prefix icu4c) \
  -DCMAKE_BUILD_TYPE=Debug \
  -DPLAYER_MACOS_BUNDLE=OFF \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build build -j