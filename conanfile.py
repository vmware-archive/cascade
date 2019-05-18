import conans

class Cascade(conans.ConanFile):
    name = 'cascade'
    version = '0.0.1'
    generators = 'cmake'
    requires = ('gtest/1.8.1@bincrafters/stable', 'ncurses/6.1@conan/stable', 'google-benchmark/1.4.1@mpusz/stable')