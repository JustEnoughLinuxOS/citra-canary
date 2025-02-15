#!/bin/bash -ex

# Determine the full revision name.
GITDATE="`git show -s --date=short --format='%ad' | sed 's/-//g'`"
GITREV="`git show -s --format='%h'`"
REV_NAME="citra-$OS-$TARGET-$GITDATE-$GITREV"

# Determine the name of the release being built.
if [[ "$GITHUB_REF_NAME" =~ ^canary- ]] || [[ "$GITHUB_REF_NAME" =~ ^nightly- ]]; then
    RELEASE_NAME=$(echo $GITHUB_REF_NAME | cut -d- -f1)
else
    RELEASE_NAME=head
fi

# Archive and upload the artifacts.
mkdir artifacts

function pack_artifacts() {
    ARTIFACTS_PATH="$1"

    # Set up root directory for archive.
    mkdir "$REV_NAME"
    if [ -f "$ARTIFACTS_PATH" ]; then
        mv "$ARTIFACTS_PATH" "$REV_NAME"

        # Use file extension to differentiate archives.
        FILENAME=$(basename "$ARTIFACT")
        EXTENSION="${FILENAME##*.}"
        ARCHIVE_NAME="$REV_NAME.$EXTENSION"
    else
        mv "$ARTIFACTS_PATH"/* "$REV_NAME"

        ARCHIVE_NAME="$REV_NAME"
    fi

    # Create .zip/.tar.gz
    if [ "$OS" = "windows" ]; then
        ARCHIVE_FULL_NAME="$ARCHIVE_NAME.zip"
        powershell Compress-Archive "$REV_NAME" "$ARCHIVE_FULL_NAME"
    elif [ "$OS" = "android" ]; then
        ARCHIVE_FULL_NAME="$ARCHIVE_NAME.zip"
        zip -r "$ARCHIVE_FULL_NAME" "$REV_NAME"
    else
        ARCHIVE_FULL_NAME="$ARCHIVE_NAME.tar.gz"
        tar czvf "$ARCHIVE_FULL_NAME" "$REV_NAME"
    fi
    mv "$ARCHIVE_FULL_NAME" artifacts/

    if [ -z "$SKIP_7Z" ]; then
        # Create .7z
        ARCHIVE_FULL_NAME="$ARCHIVE_NAME.7z"
        mv "$REV_NAME" "$RELEASE_NAME"
        7z a "$ARCHIVE_FULL_NAME" "$RELEASE_NAME"
        mv "$ARCHIVE_FULL_NAME" artifacts/

        # Clean up created release artifacts directory.
        rm -rf "$RELEASE_NAME"
    else
        # Clean up created rev artifacts directory.
        rm -rf "$REV_NAME"
    fi
}

if [ -z "$PACK_INDIVIDUALLY" ]; then
    # Pack all of the artifacts at once.
    pack_artifacts build/bundle
else
    # Pack and upload the artifacts one-by-one.
    for ARTIFACT in build/bundle/*; do
        pack_artifacts "$ARTIFACT"
    done
fi
