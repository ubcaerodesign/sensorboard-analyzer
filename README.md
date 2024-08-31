# sensorboard-analyzer
App for analyzing values on the sensorboard graphically as well as calibrating some sensors.

The project is built using premake5 and Visual Studio. So if you want to change project configuration settings and build a release you need to have those installed.

Then u can run
premake5 vs2022
msbuild MyProject.sln /p:Configuration=Debug /p:Platform=x64
