#!/bin/bash
SUDO=sudo
usage(){
	echo "$0 [DEVEL|PROD|TEST|API]"
}


if [ "$#" != "1" ];then
	usage >> /dev/stderr
	exit 1
fi

if [ -z "$MCSTAS" ];then
    PREFIX=/usr/local/
    export MCSTAS=${PREFIX}/mcstas/2.6.1
    export MCSTAS_TOOLS=$PREFIX/mcstas/2.6.1/tools/Perl/
    export PATH=$PREFIX/mcstas/2.6.1/miniconda3/bin:$PREFIX/mcstas/2.6.1/bin:$PATH
fi

current_branch=`git status --porcelain -b | head -1 | cut -d ' ' -f 2 | sed 's|\..*||g'`
mkdir build/{DEVEL,PROD,TEST,API} -p
case $1 in
	DEVEL)
		OPTS="-DSERVER=True -DDO_INSTRUMENTS=True -DCMAKE_INSTALL_PREFIX=/tmp/devel/"
		unset SUDO
		;;
	PROD)
		OPTS="-DSERVER=True -DDO_INSTRUMENTS=True -DCMAKE_INSTALL_PREFIX=/usr/local/"
		;;
	TEST)
		newbranch=TEST
		OPTS="-DSERVER=True -DDO_INSTRUMENTS=True -DCMAKE_INSTALL_PREFIX=/usr/local/"
		;;
	API)
		OPTS=""
		;;
	*)
		echo "[ERROR] option $1 not recognized" >> /dev/stderr
		usage >> /dev/stderr
		exit 1
		;;
esac


if [ -n "$newbranch" ];then
	git stash
	git branch $newbranch --track origin/master || exit 1
	git checkout $newbranch || exit 1
fi

cd build/$1
cmake -DCMAKE_BUILD_TYPE=Debug $OPTS -DMCSTAS=${MCSTAS} ../../
cmake --build . || exit 1
make
ctest --output-on-failure && $SUDO make install

if [ -n "$newbranch" ]; then
	git checkout $current_branch
	git branch -d $newbranch
	git stash pop
fi

