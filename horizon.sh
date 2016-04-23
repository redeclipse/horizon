#!/bin/sh
if [ "${HORIZON_CALLED}" = "true" ]; then HORIZON_EXITR="return"; else HORIZON_EXITR="exit"; fi
HORIZON_SCRIPT="$0"
HORIZON_ARGS=$@
HORIZON_SYSTEM="$(uname -s)"
if [ -z "${HORIZON_HOME+isset}" ]; then HORIZON_HOME="${HOME}/.horizon"; fi

horizon_path() {
    if [ -z "${HORIZON_PATH+isset}" ]; then HORIZON_PATH="$(cd "$(dirname "$0")" && pwd)"; fi
}

horizon_init() {
    if [ -z "${HORIZON_BINARY+isset}" ]; then HORIZON_BINARY="horizon"; fi
    HORIZON_SUFFIX=""
    HORIZON_MAKE="make -C src install"
}

horizon_setup() {
    if [ -z "${HORIZON_TARGET+isset}" ]; then
        HORIZON_MACHINE="$(uname -m)"
        case "${HORIZON_SYSTEM}" in
            Linux)
                HORIZON_SUFFIX="_linux"
                HORIZON_TARGET="linux"
                ;;
            Darwin)
		        HORIZON_SUFFIX="_universal"
		        HORIZON_TARGET="macosx"
                HORIZON_ARCH="horizon.app/Contents/MacOS"
                HORIZON_MAKE="./src/osxbuild.sh all install"
		;;
            FreeBSD)
                HORIZON_SUFFIX="_bsd"
                HORIZON_TARGET="bsd"
                HORIZON_BRANCH="source" # we don't have binaries for bsd yet sorry
                ;;
            MINGW*)
                HORIZON_SUFFIX=".exe"
                HORIZON_TARGET="windows"
                HORIZON_MAKE="mingw32-make"
                if [ -n "${PROCESSOR_ARCHITEW6432+isset}" ]; then
                    HORIZON_MACHINE="${PROCESSOR_ARCHITEW6432}"
                else
                    HORIZON_MACHINE="${PROCESSOR_ARCHITECTURE}"
                fi
                ;;
            *)
                echo "Unsupported system: ${HORIZON_SYSTEM}"
                return 1
                ;;
        esac
    fi
    if [ -z "${HORIZON_ARCH+isset}" ] && [ "${HORIZON_TARGET}" != "macosx" ]; then
        case "${HORIZON_MACHINE}" in
            i486|i586|i686|x86)
                HORIZON_ARCH="x86"
                ;;
            x86_64|[Aa][Mm][Dd]64)
                HORIZON_ARCH="amd64"
                ;;
            arm|armv*)
                HORIZON_ARCH="arm"
                ;;
            *)
                HORIZON_ARCH="native"
                ;;
        esac
    fi
    if [ -e "${HORIZON_PATH}/branch.txt" ]; then HORIZON_BRANCH_CURRENT=`cat "${HORIZON_PATH}/branch.txt"`; fi
    if [ -z "${HORIZON_BRANCH+isset}" ]; then
        if [ -n "${HORIZON_BRANCH_CURRENT+isset}" ]; then
            HORIZON_BRANCH="${HORIZON_BRANCH_CURRENT}"
        elif [ -e ".git" ]; then
            HORIZON_BRANCH="devel"
        else
            HORIZON_BRANCH="stable"
        fi
    fi
    if [ -z "${HORIZON_HOME+isset}" ] && [ "${HORIZON_BRANCH}" != "stable" ] && [ "${HORIZON_BRANCH}" != "inplace" ]; then HORIZON_HOME="home"; fi
    if [ -n "${HORIZON_HOME+isset}" ]; then HORIZON_OPTIONS="-u${HORIZON_HOME} ${HORIZON_OPTIONS}"; fi
    horizon_check
    return $?
}

horizon_check() {
    if [ "${HORIZON_BRANCH}" = "source" ]; then
        echo ""
        if [ -n "${HORIZON_MAKE}" ]; then
            echo "Rebuilding \"${HORIZON_BRANCH}\". To disable set: HORIZON_BRANCH=\"inplace\""
            echo ""
            ${HORIZON_MAKE}
        else
            echo "Unable to build \"${HORIZON_BRANCH}\". Using: HORIZON_BRANCH=\"devel\""
            echo ""
            HORIZON_BRANCH="devel"
        fi
    fi
    if [ "${HORIZON_BRANCH}" != "inplace" ] && [ "${HORIZON_BRANCH}" != "source" ]; then
        echo ""
        echo "Checking for updates to \"${HORIZON_BRANCH}\". To disable set: HORIZON_BRANCH=\"inplace\""
        echo ""
        horizon_begin
        return $?
    fi
    horizon_runit
    return $?
}

horizon_begin() {
    HORIZON_RETRY="false"
    horizon_update
    return $?
}

horizon_retry() {
    if [ "${HORIZON_RETRY}" != "true" ]; then
        HORIZON_RETRY="true"
        echo "Retrying..."
        horizon_update
        return $?
    fi
    horizon_runit
    return $?
}

horizon_update() {
    chmod +x "${HORIZON_PATH}/bin/update.sh"
    HORIZON_CALLED="true" . "${HORIZON_PATH}/bin/update.sh"
    if [ $? -eq 0 ]; then
        horizon_runit
        return $?
    else
        horizon_retry
        return $?
    fi
    return 0
}

horizon_runit() {
    export HORIZON_TARGET
    export HORIZON_BRANCH
    export HORIZON_ARCH
    export HORIZON_MACHINE
    export HORIZON_BINARY
    export HORIZON_SUFFIX
    export HORIZON_PATH
    if [ -e "${HORIZON_PATH}/bin/${HORIZON_ARCH}/${HORIZON_BINARY}${HORIZON_SUFFIX}" ]; then
        HORIZON_PWD=`pwd`
        export HORIZON_PWD
        cd "${HORIZON_PATH}" || return 1
        exec "${HORIZON_PATH}/bin/${HORIZON_ARCH}/${HORIZON_BINARY}${HORIZON_SUFFIX}" ${HORIZON_OPTIONS} ${HORIZON_ARGS} || (
            cd "${HORIZON_PWD}"
            return 1
        )
        cd "${HORIZON_PWD}"
        return 0
    else
        if [ "${HORIZON_BRANCH}" = "source" ]; then
            if [ -n "${HORIZON_MAKE}" ]; then
                ${HORIZON_MAKE} && ( horizon_runit; return $? )
            fi
            HORIZON_BRANCH="devel"
        fi
        if [ "${HORIZON_BRANCH}" != "inplace" ] && [ "${HORIZON_TRYUPDATE}" != "true" ]; then
            HORIZON_TRYUPDATE="true"
            horizon_begin
            return $?
        fi
        if [ "${HORIZON_ARCH}" = "amd64" ]; then
            HORIZON_ARCH="x86"
            horizon_runit
            return $?
        elif [ "${HORIZON_ARCH}" = "x86" ]; then
            HORIZON_ARCH="native"
            horizon_runit
            return $?
        fi
        echo "Unable to find a working binary."
    fi
    return 1
}

horizon_path
horizon_init
horizon_setup

if [ $? -ne 0 ]; then
    echo ""
    echo "There was an error running Red Eclipse Horizon."
    ${HORIZON_EXITR} 1
else
    ${HORIZON_EXITR} 0
fi
