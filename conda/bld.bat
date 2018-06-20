set PYTHON_VERSION=3.6
call conda info
call conda config --set anaconda_upload yes
REM call conda build --python %PYTHON_VERSION% -c conda-forge -c mhochsteger --user mhochsteger --token %ANACONDA_TOKEN% ngsolve_gui
