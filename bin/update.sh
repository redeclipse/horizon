#!/bin/sh
if [ "${HORIZON_CALLED}" = "true" ]; then HORIZON_EXITU="return"; else HORIZON_EXITU="exit"; fi
if [ "${HORIZON_DEPLOY}" != "true" ]; then HORIZON_DEPLOY="false"; fi
HORIZON_SCRIPT="$0"

horizon_update_path() {
    if [ -z "${HORIZON_PATH+isset}" ]; then HORIZON_PATH="$(cd "$(dirname "$0")" && cd .. && pwd)"; fi
}

horizon_update_init() {
    if [ -z "${HORIZON_SOURCE+isset}" ]; then HORIZON_SOURCE="http://redeclipse.net/horizon"; fi
    if [ -z "${HORIZON_GITHUB+isset}" ]; then HORIZON_GITHUB="https://github.com/red-eclipse"; fi
    if [ -z "${HORIZON_CACHE+isset}" ]; then
        if [ "${HORIZON_TARGET}" = "windows" ]; then
            HORIZON_WINDOCS=`reg query "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders" //v "Personal" | tr -d '\r' | tr -d '\n' | sed -e 's/.*\(.\):\\\/\/\1\//g;s/\\\/\//g'`
            if [ -d "${HORIZON_WINDOCS}" ]; then
                HORIZON_CACHE="${HORIZON_WINDOCS}/My Games/Red Eclipse Horizon/cache"
                return 0
            fi
        elif [ "${HORIZON_TARGET}" = "macosx" ]; then
            HORIZON_CACHE="${HOME}/Library/Application Support/Red Eclipse Horizon/cache"
        else
            HORIZON_CACHE="${HOME}/.horizon/cache"
        fi
    fi
    return 0
}

horizon_update_setup() {
    if [ -z "${HORIZON_TARGET+isset}" ]; then
        HORIZON_SYSTEM="$(uname -s)"
        HORIZON_MACHINE="$(uname -m)"
        case "${HORIZON_SYSTEM}" in
            Linux)
                HORIZON_TARGET="linux"
                ;;
            Darwin)
                HORIZON_TARGET="macosx"
                ;;
            FreeBSD)
                HORIZON_TARGET="bsd"
                HORIZON_BRANCH="source" # we don't have binaries for bsd yet sorry
                ;;
            MINGW*)
                HORIZON_TARGET="windows"
                ;;
            *)
                echo "Unsupported system: ${HORIZON_SYSTEM}"
                return 1
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
    HORIZON_UPDATE="${HORIZON_BRANCH}"
    HORIZON_TEMP="${HORIZON_CACHE}/${HORIZON_BRANCH}"
    case "${HORIZON_TARGET}" in
        windows)
            HORIZON_BLOB="zipball"
            HORIZON_ARCHIVE="windows.zip"
            HORIZON_ARCHEXT="zip"
            ;;
        linux)
            HORIZON_BLOB="tarball"
            HORIZON_ARCHIVE="linux.tar.gz"
            HORIZON_ARCHEXT="tar.gz"
            ;;
        macosx)
            HORIZON_BLOB="tarball"
            HORIZON_ARCHIVE="macosx.tar.gz"
            HORIZON_ARCHEXT="tar.gz"
            ;;
        *)
            echo "Unsupported update target: ${HORIZON_SYSTEM}"
            return 1
            ;;
    esac
    horizon_update_branch
    return $?
}

horizon_update_branch() {
    echo "branch: ${HORIZON_UPDATE}"
    echo "folder: ${HORIZON_PATH}"
    echo "cached: ${HORIZON_TEMP}"
    if [ -z `which curl` ]; then
        if [ -z `which wget` ]; then
            echo "Unable to find curl or wget, are you sure you have one installed?"
            return 1
        else
            HORIZON_DOWNLOADER="wget --no-check-certificate -U \"horizon-${HORIZON_UPDATE}\" -O"
        fi
    else
        HORIZON_DOWNLOADER="curl -L -k -f -A \"horizon-${HORIZON_UPDATE}\" -o"
    fi
    if [ "${HORIZON_BLOB}" = "zipball" ]; then
        if [ -z `which unzip` ]; then
            echo "Unable to find unzip, are you sure you have it installed?"
            return 1
        fi
        HORIZON_UNZIP="unzip -o"
    fi
    if [ -z `which tar` ]; then
        echo "Unable to find tar, are you sure you have it installed?"
        return 1
    fi
    HORIZON_TAR="tar -xv"
    if ! [ -d "${HORIZON_TEMP}" ]; then mkdir -p "${HORIZON_TEMP}"; fi
    echo "#"'!'"/bin/sh" > "${HORIZON_TEMP}/install.sh"
    if [ "${HORIZON_BRANCH}" = "devel" ]; then
        horizon_update_bins_run
        return $?
    fi
    horizon_update_module
    return $?
}

horizon_update_module() {
    echo "modules: Updating.."
    ${HORIZON_DOWNLOADER} "${HORIZON_TEMP}/mods.txt" "${HORIZON_SOURCE}/${HORIZON_UPDATE}/mods.txt"
    if ! [ -e "${HORIZON_TEMP}/mods.txt" ]; then
        echo "modules: Failed to retrieve update information."
        horizon_update_bins_run
        return $?
    fi
    HORIZON_MODULE_LIST=`cat "${HORIZON_TEMP}/mods.txt"`
    if [ -z "${HORIZON_MODULE_LIST}" ]; then
        echo "modules: Failed to get list, continuing.."
    else
        if [ -d "${HORIZON_TEMP}/data.txt" ]; then rm -rfv "${HORIZON_TEMP}/data.txt"; fi
        if [ -d "${HORIZON_TEMP}/data.zip" ]; then rm -rfv "${HORIZON_TEMP}/data.zip"; fi
        if [ -d "${HORIZON_TEMP}/data.tar.gz" ]; then rm -rfv "${HORIZON_TEMP}/data.tar.gz"; fi
        echo "modules: Prefetching versions.."
        HORIZON_MODULE_PREFETCH=""
        for a in ${HORIZON_MODULE_LIST}; do
            rm -f "${HORIZON_TEMP}/${a}.txt"
            if [ -n "${HORIZON_MODULE_PREFETCH}" ]; then
                HORIZON_MODULE_PREFETCH="${HORIZON_MODULE_PREFETCH},${a}"
            else
                HORIZON_MODULE_PREFETCH="${a}"
            fi
        done
        if [ -n "${HORIZON_MODULE_PREFETCH}" ]; then
            ${HORIZON_DOWNLOADER} "${HORIZON_TEMP}/#1.txt" "${HORIZON_SOURCE}/${HORIZON_UPDATE}/{${HORIZON_MODULE_PREFETCH}}.txt"
            for a in ${HORIZON_MODULE_LIST}; do
                HORIZON_MODULE_RUN="${a}"
                if [ -n "${HORIZON_MODULE_RUN}" ]; then
                    horizon_update_module_run
                    if [ $? -ne 0 ]; then
                        echo "${HORIZON_MODULE_RUN}: There was an error updating the module, continuing.."
                    fi
                fi
            done
        else
            echo "modules: Failed to get version information, continuing.."
        fi
    fi
    horizon_update_bins_run
    return $?
}

horizon_update_module_run() {
    if [ "${HORIZON_MODULE_RUN}" = "horizon" ]; then
        HORIZON_MODULE_DIR=""
    else
        HORIZON_MODULE_DIR="/data/${HORIZON_MODULE_RUN}"
    fi
    if  [ -e "${HORIZON_PATH}${HORIZON_MODULE_DIR}/version.txt" ]; then
        horizon_update_module_ver
        return $?
    fi
    echo "${HORIZON_MODULE_RUN}: Unable to find version.txt. Will start from scratch."
    HORIZON_MODULE_INSTALLED="none"
    echo "mkdir -p \"${HORIZON_PATH}${HORIZON_MODULE_DIR}\"" >> "${HORIZON_TEMP}/install.sh"
    horizon_update_module_get
    return $?
}

horizon_update_module_ver() {
    if [ -e "${HORIZON_PATH}${HORIZON_MODULE_DIR}/version.txt" ]; then HORIZON_MODULE_INSTALLED=`cat "${HORIZON_PATH}${HORIZON_MODULE_DIR}/version.txt"`; fi
    if [ -z "${HORIZON_MODULE_INSTALLED}" ]; then HORIZON_MODULE_INSTALLED="none"; fi
    echo "${HORIZON_MODULE_RUN}: ${HORIZON_MODULE_INSTALLED} is installed."
    horizon_update_module_get
    return $?
}

horizon_update_module_get() {
    if ! [ -e "${HORIZON_TEMP}/${HORIZON_MODULE_RUN}.txt" ]; then
        echo "${HORIZON_MODULE_RUN}: Failed to retrieve update information."
        return $?
    fi
    HORIZON_MODULE_REMOTE=`cat "${HORIZON_TEMP}/${HORIZON_MODULE_RUN}.txt"`
    if [ -z "${HORIZON_MODULE_REMOTE}" ]; then
        echo "${HORIZON_MODULE_RUN}: Failed to read update information."
        return $?
    fi
    echo "${HORIZON_MODULE_RUN}: ${HORIZON_MODULE_REMOTE} is the current version."
    if [ "${HORIZON_MODULE_REMOTE}" = "${HORIZON_MODULE_INSTALLED}" ]; then
        echo "echo \"${HORIZON_MODULE_RUN}: already up to date.\"" >> "${HORIZON_TEMP}/install.sh"
        return $?
    fi
    horizon_update_module_blob
    return $?
}

horizon_update_module_blob() {
    if [ -e "${HORIZON_TEMP}/${HORIZON_MODULE_RUN}.${HORIZON_ARCHEXT}" ]; then
        rm -f "${HORIZON_TEMP}/${HORIZON_MODULE_RUN}.${HORIZON_ARCHEXT}"
    fi
    echo "${HORIZON_MODULE_RUN}: Downloading ${HORIZON_GITHUB}/${HORIZON_MODULE_RUN}/${HORIZON_BLOB}/${HORIZON_MODULE_REMOTE}"
    ${HORIZON_DOWNLOADER} "${HORIZON_TEMP}/${HORIZON_MODULE_RUN}.${HORIZON_ARCHEXT}" "${HORIZON_GITHUB}/${HORIZON_MODULE_RUN}/${HORIZON_BLOB}/${HORIZON_MODULE_REMOTE}"
    if ! [ -e "${HORIZON_TEMP}/${HORIZON_MODULE_RUN}.${HORIZON_ARCHEXT}" ]; then
        echo "${HORIZON_MODULE_RUN}: Failed to retrieve update package."
        return $?
    fi
    horizon_update_module_blob_deploy
    return $?
}

horizon_update_module_blob_deploy() {
    echo "echo \"${HORIZON_MODULE_RUN}: deploying blob.\"" >> "${HORIZON_TEMP}/install.sh"
    if [ "${HORIZON_BLOB}" = "zipball" ]; then
        echo "${HORIZON_UNZIP} -o \"${HORIZON_TEMP}/${HORIZON_MODULE_RUN}.${HORIZON_ARCHEXT}\" -d \"${HORIZON_TEMP}\" && (" >> "${HORIZON_TEMP}/install.sh"
    else
        echo "${HORIZON_TAR} -f \"${HORIZON_TEMP}/${HORIZON_MODULE_RUN}.${HORIZON_ARCHEXT}\" -C \"${HORIZON_TEMP}\" && (" >> "${HORIZON_TEMP}/install.sh"
    fi
    if [ "${HORIZON_MODULE_RUN}" != "horizon" ]; then
        echo "   rm -rf \"${HORIZON_PATH}${HORIZON_MODULE_DIR}\"" >> "${HORIZON_TEMP}/install.sh"
        echo "   mkdir -p \"${HORIZON_PATH}${HORIZON_MODULE_DIR}\"" >> "${HORIZON_TEMP}/install.sh"
    fi
    echo "   cp -Rfv \"${HORIZON_TEMP}/red-eclipse-${HORIZON_MODULE_RUN}-$(echo "$HORIZON_MODULE_REMOTE" | cut -b 1-7)/\"* \"${HORIZON_PATH}${HORIZON_MODULE_DIR}\"" >> "${HORIZON_TEMP}/install.sh"
    echo "   rm -rf \"${HORIZON_TEMP}/red-eclipse-${HORIZON_MODULE_RUN}-$(echo "$HORIZON_MODULE_REMOTE" | cut -b 1-7)\"" >> "${HORIZON_TEMP}/install.sh"
    echo "   echo \"${HORIZON_MODULE_REMOTE}\" > \"${HORIZON_PATH}${HORIZON_MODULE_DIR}/version.txt\"" >> "${HORIZON_TEMP}/install.sh"
    echo ") || (" >> "${HORIZON_TEMP}/install.sh"
    echo "    rm -f \"${HORIZON_TEMP}/${HORIZON_MODULE_RUN}.txt\"" >> "${HORIZON_TEMP}/install.sh"
    echo "    exit 1" >> "${HORIZON_TEMP}/install.sh"
    echo ")" >> "${HORIZON_TEMP}/install.sh"
    HORIZON_DEPLOY="true"
    return $?
}

horizon_update_bins_run() {
    echo "bins: Updating.."
    rm -f "${HORIZON_TEMP}/bins.txt"
    ${HORIZON_DOWNLOADER} "${HORIZON_TEMP}/bins.txt" "${HORIZON_SOURCE}/${HORIZON_UPDATE}/bins.txt"
    if [ -e "${HORIZON_PATH}/bin/version.txt" ]; then HORIZON_BINS=`cat "${HORIZON_PATH}/bin/version.txt"`; fi
    if [ -z "${HORIZON_BINS}" ]; then HORIZON_BINS="none"; fi
    echo "bins: ${HORIZON_BINS} is installed."
    horizon_update_bins_get
    return $?
}

horizon_update_bins_get() {
    if ! [ -e "${HORIZON_TEMP}/bins.txt" ]; then
        echo "bins: Failed to retrieve update information."
        horizon_update_deploy
        return $?
    fi
    HORIZON_BINS_REMOTE=`cat "${HORIZON_TEMP}/bins.txt"`
    if [ -z "${HORIZON_BINS_REMOTE}" ]; then
        echo "bins: Failed to read update information."
        horizon_update_deploy
        return $?
    fi
    echo "bins: ${HORIZON_BINS_REMOTE} is the current version."
    if [ "${HORIZON_TRYUPDATE}" != "true" ] && [ "${HORIZON_BINS_REMOTE}" = "${HORIZON_BINS}" ]; then
        echo "echo \"bins: already up to date.\"" >> "${HORIZON_TEMP}/install.sh"
        horizon_update_deploy
        return $?
    fi
    horizon_update_bins_blob
    return $?
}

horizon_update_bins_blob() {
    if [ -e "${HORIZON_TEMP}/${HORIZON_ARCHIVE}" ]; then
        rm -f "${HORIZON_TEMP}/${HORIZON_ARCHIVE}"
    fi
    echo "bins: Downloading ${HORIZON_SOURCE}/${HORIZON_UPDATE}/${HORIZON_ARCHIVE}"
    ${HORIZON_DOWNLOADER} "${HORIZON_TEMP}/${HORIZON_ARCHIVE}" "${HORIZON_SOURCE}/${HORIZON_UPDATE}/${HORIZON_ARCHIVE}"
    if ! [ -e "${HORIZON_TEMP}/${HORIZON_ARCHIVE}" ]; then
        echo "bins: Failed to retrieve bins update package."
        horizon_update_deploy
        return $?
    fi
    horizon_update_bins_deploy
    return $?
}

horizon_update_bins_deploy() {
    echo "echo \"bins: deploying blob.\"" >> "${HORIZON_TEMP}/install.sh"
    if [ "${HORIZON_TARGET}" = "windows" ]; then
        echo "${HORIZON_UNZIP} -o \"${HORIZON_TEMP}/${HORIZON_ARCHIVE}\" -d \"${HORIZON_PATH}\" && (" >> "${HORIZON_TEMP}/install.sh"
    else
        echo "${HORIZON_TAR} -f \"${HORIZON_TEMP}/${HORIZON_ARCHIVE}\" -C \"${HORIZON_PATH}\" && (" >> "${HORIZON_TEMP}/install.sh"
    fi
    echo "    echo \"${HORIZON_BINS_REMOTE}\" > \"${HORIZON_PATH}/bin/version.txt\"" >> "${HORIZON_TEMP}/install.sh"
    echo ") || (" >> "${HORIZON_TEMP}/install.sh"
    echo "    rm -f \"${HORIZON_TEMP}/bins.txt\"" >> "${HORIZON_TEMP}/install.sh"
    echo "    exit 1" >> "${HORIZON_TEMP}/install.sh"
    echo ")" >> "${HORIZON_TEMP}/install.sh"
    HORIZON_DEPLOY="true"
    horizon_update_deploy
    return $?
}

horizon_update_deploy() {
    if [ "${HORIZON_DEPLOY}" != "true" ]; then return 0; fi
    echo "deploy: \"${HORIZON_TEMP}/install.sh\""
    chmod ugo+x "${HORIZON_TEMP}/install.sh"
    HORIZON_INSTALL="exec"
    touch test.tmp && (
        rm -f test.tmp
        horizon_update_unpack
        return $?
    ) || (
        echo "Administrator permissions are required to deploy the files."
        if [ -z `which sudo` ]; then
            echo "Unable to find sudo, are you sure it is installed?"
            return 1
        else
            HORIZON_INSTALL="sudo exec"
            horizon_update_unpack
            return $?
        fi
    )
    return $?
}

horizon_update_unpack() {
    ${HORIZON_INSTALL} "${HORIZON_TEMP}/install.sh" && (
        echo "${HORIZON_BRANCH}" > "${HORIZON_PATH}/branch.txt"
        return 0
    ) || (
        echo "There was an error deploying the files."
        return 1
    )
}

horizon_update_path
horizon_update_init
horizon_update_setup

if [ $? -ne 0 ]; then
    ${HORIZON_EXITU} 1
else
    ${HORIZON_EXITU} 0
fi
