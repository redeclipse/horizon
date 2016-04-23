#!/bin/sh
while [ -n "$1" ]; do
 case "$1" in
  clean)
   xcodebuild -project src/xcode/horizon.xcodeproj clean -configuration Release
   ;;
  all)
   xcodebuild -project src/xcode/horizon.xcodeproj -configuration Release -alltargets
   ;;
  install)
   cp -v src/xcode/build/Release/horizon.app/Contents/MacOS/horizon bin/horizon.app/Contents/MacOS/horizon_universal
   chmod +x bin/horizon.app/Contents/MacOS/horizon_universal
   ;;
 esac
 shift
done
