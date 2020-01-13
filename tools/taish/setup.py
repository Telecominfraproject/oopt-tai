import setuptools
import subprocess as sp
import os

install_requires = ['grpcio-tools', 'prompt_toolkit', 'tabulate', 'grpclib']

os.chdir(os.path.dirname(os.path.abspath(__file__)))

with open('README.md', 'r') as f:
    long_description = f.read()

cmd = 'make -C ./client'
sp.run(cmd.split())

# list tags on the current branch
cmd = 'git log --decorate --simplify-by-decoration --pretty=oneline HEAD --format=%d'
proc = sp.Popen(cmd.split(), stdout=sp.PIPE)

version = None
for line in iter(proc.stdout.readline, b''):
    s = line.decode('utf-8').strip()
    for v in s.split(','):
        l = v.strip('()').strip()
        if l.startswith('tag: v'):
            version = l.split()[1][1:]
            break;
    if version:
        break

if version == None:
    version = '1.0a1'

setuptools.setup(
        name='taish',
        version=version,
        install_requires=install_requires,
        description='TAI shell',
        long_description=long_description,
        long_description_content_type='text/markdown',
        url='https://github.com/Telecominfraproject/oopt-tai/tree/master/tools/taish',
        package_dir={'': 'client'},
        packages=setuptools.find_namespace_packages(where='client'),
        python_requires='>=3.7',
        entry_points={
            'console_scripts': [
                'taish = taish.main:main',
            ],
        },
)
