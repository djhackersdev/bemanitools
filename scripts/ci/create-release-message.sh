#!/bin/sh

VERSION="$1"

section_active=""
changelog_excerpt=""

while IFS= read -r line; do
    if [ "$section_active" ]; then
        if [[ "$(echo "$line" | grep '^#')" ]]; then
            section_active=""
        else
            changelog_excerpt="$(printf "%s\n%s" "$changelog_excerpt" "$line")"
        fi
    else
        if [ "$line" = "## $VERSION" ]; then
            section_active="1"
        fi
    fi
done

if [ "$changelog_excerpt" ]; then
    printf "%s" "$changelog_excerpt"
    exit 0
else
    >&2 echo "Could not find version in changelog: $VERSION"
    exit 1
fi