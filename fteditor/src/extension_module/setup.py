from distutils.core import setup, Extension

def main():
    setup(name='interact',
          version='1.0.0',
          description='Python interface for interacting with the C library function',
          author='liem.do',
          author_email='liem.do@brtchip.com',
          ext_modules=[Extension('interact', 
                ['InteractModule.cpp'], 
                extra_compile_args = ['/std:c++17'])])

if __name__ == "__main__":
    main()