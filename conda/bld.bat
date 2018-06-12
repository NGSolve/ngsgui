set PYTHON_VERSION=3.6
call conda info
call conda build --python %PYTHON_VERSION% -c conda-forge -c mhochsteger ngsolve_gui
