from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

ext_modules = [
    Pybind11Extension(
        "sistema_recomendacao",
        [
            "bindings.cpp",
            "similaridade.cpp",
            "recomendacao.cpp",
            "recomendacao_csr.cpp",
            "csr.cpp",
        ],
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