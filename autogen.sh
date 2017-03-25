set -ex
# don't mind this weird-looking portability hack
env=$(which env)
#!env
#
# A bootstrapping script that can be used to generate the autoconf 
# and automake-related scripts of the build process.

autoreconf --install --force --Warn all

