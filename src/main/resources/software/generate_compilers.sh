#!/bin/bash

##
# Script to build GCC for microblaze (originally) now for any triple.
# Written by Martijn Koedam (m.l.p.j.koedam@tue.nl)
# Mod by Deano Calver
#
# Current version is tested on ubuntu 20
##


CORES=8
DISTCLEAN=0

GCC_URL=ftp://ftp.nluug.nl/mirror/languages/gcc/releases/gcc-10.2.0/gcc-10.2.0.tar.xz
BINUTILS_URL=http://ftp.gnu.org/gnu/binutils/binutils-2.35.1.tar.xz
GCC_FILE=$(basename $GCC_URL)
BINUTILS_FILE=$(basename $BINUTILS_URL)

GCC=${GCC_FILE%.tar.*}
BINUTILS=${BINUTILS_FILE%.tar.*}

function download()
{
	if [ ! -f $1 ]
	then
	  wget -O $1 $2
	else
	  echo "$1 exists"
	fi
}

function extract()
{
	if [ ! -d "$2/$3" ]
	then
	  tar xf $1 -C $2
	fi

	if [ ! -d "$2/$3" ]
	then
	  echo "Failed to extract $1 to $2/$3"
	  exit 1
	fi
}

if [ ! -f $PWD/build/compilers ]
then
 mkdir -p $PWD/build/compilers
fi

#downloading and extracting files.
echo "Downloading"
download "build/compilers/$GCC_FILE" "$GCC_URL"
download "build/compilers/$BINUTILS_FILE" "$BINUTILS_URL"
if [ ! -d "build/compilers/src" ]
then
 mkdir -p "build/compilers/src"
fi
extract "build/compilers/$BINUTILS_FILE" "build/compilers/src" "$BINUTILS"
extract "build/compilers/$GCC_FILE" "build/compilers/src" "$GCC"

pushd "build/compilers/src/$GCC" || exit 1
contrib/download_prerequisites
popd || exit 1

function prep_build()
{
	if [ ! -d "build/compilers/$2/$1" ]
	then
	  mkdir -p "build/compilers/$2/$1"
	  pushd build/compilers/$2/$1 || exit 1
	else
	  pushd build/compilers/$2/$1 || exit 1
	  if [ $5 != 0 ]
	  then
	    make distclean
	  fi
	fi

 ../../src/$1/configure --target=$2 --program-prefix=$2- --prefix=$3/$2 $4
	if [ $? != 0 ]
	then
	  echo "Failed to configure"
	  exit 1
	fi

	popd || exit 1
}

function build()
{
	pushd "build/compilers/$2/$1" || exit 1

	make -j"$CORES" all"$3" CFLAGS_FOR_TARGET="$4"
	echo "$4"
	if [ $? != 0 ]
	then
	  echo "Failed to build"
	  exit 1
	fi
	make install$3
	if [ $? != 0 ]
	then
	  echo "Failed to install"
	  exit 1
	fi

	popd || exit 1
}


function build_binutils()
{
	echo "Building binutils"
	prep_build $BINUTILS $1 $2 "" $DISTCLEAN
	build $BINUTILS $1 ""

	# put results into PATH.
	export PATH=$PATH:$2/$1/bin/
}

function build_gcc()
{
	prep_build $GCC $1 $2 "--enable-languages=c,c++ --disable-nls --without-headers --disable-multilib --disable-libssp --with-endian=little" $DISTCLEAN
	build $GCC $1 "-host"
	build $GCC $1 "-gcc"
	build $GCC $1 "-target-libgcc" "$3"
}