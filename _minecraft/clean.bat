del *.sdf
del *.ncb
del /A:H *.suo
rmdir /Q /S Debug
rmdir /Q /S Release
rmdir /Q /S "base\Debug"
rmdir /Q /S "base\Release"
rmdir /Q /S ipch
pause
