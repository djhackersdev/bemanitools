#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

set -e

cd $DIR

echo "Running tests..."

wine ./cconfig-test.exe
wine ./cconfig-util-test.exe
wine ./cconfig-cmd-test.exe
wine ./iidxhook-util-config-eamuse-test.exe
wine ./iidxhook-util-config-gfx-test.exe
# wine ./iidxhook-config-iidxhook1-test.exe
# wine ./iidxhook-config-iidxhook2-test.exe
wine ./iidxhook-util-config-misc-test.exe
wine ./iidxhook-util-config-sec-test.exe
wine ./security-id-test.exe
wine ./security-mcode-test.exe
wine ./security-util-test.exe
wine ./security-rp-test.exe
wine ./security-rp2-test.exe
wine ./security-rp3-test.exe
wine ./util-net-test.exe

wine ./inject.exe d3d9hook.dll d3d9hook-test.exe

echo "All tests successful."