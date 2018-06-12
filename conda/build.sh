set -e
PYTHON_VERSION=3.6
conda install -y conda-build>=3
conda install -y anaconda-client
CONDA_ARGS=" --python ${PYTHON_VERSION} -c conda-forge -c mhochsteger"
conda build ${CONDA_ARGS} ngsolve_gui
