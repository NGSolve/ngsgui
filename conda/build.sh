set -e
PYTHON_VERSION=3.6
conda install -y conda-build>=3
conda install -y anaconda-client
conda config --set anaconda_upload yes
CONDA_ARGS=" --python ${PYTHON_VERSION} -c conda-forge -c mhochsteger --user mhochsteger --token ${ANACONDA_TOKEN}"
# conda build ${CONDA_ARGS} ngsolve_gui
