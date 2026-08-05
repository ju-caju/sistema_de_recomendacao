from pathlib import Path

from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

diretorio_integracao = Path(__file__).resolve().parent
diretorio_raiz = diretorio_integracao.parent

ext_modules = [
    Pybind11Extension(
        "sistema_recomendacao",
        [
            str(diretorio_integracao / "bindings.cpp"),
            str(diretorio_raiz / "similaridade.cpp"),
            str(diretorio_raiz / "recomendacao.cpp"),
            str(diretorio_raiz / "recomendacao_csr.cpp"),
            str(diretorio_raiz / "csr.cpp"),
        ],
        include_dirs=[str(diretorio_raiz)],
        cxx_std=11,
    )
]

setup(
    name="sistema_recomendacao",
    version="0.1.0",
    description="Integracao Python-C++ do sistema de recomendacao",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)
